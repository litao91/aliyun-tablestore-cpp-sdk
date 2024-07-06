#include "reader.hpp"
#include "consts.hpp"
#include "tablestore/core/types.hpp"
#include "tablestore/util/security.hpp"
#include "tablestore/util/try.hpp"
#include "tablestore/util/foreach.hpp"
#include <boost/static_assert.hpp>
#include <type_traits>
#include <cstring>

using namespace std;
using namespace aliyun::tablestore::util;

namespace aliyun {
namespace tablestore {
namespace core {
namespace plainbuffer {

namespace {

std::optional<OTSError> issueError(const char* filename, int line)
{
    OTSError e(OTSError::kPredefined_CorruptedResponse);
    e.mutableMessage() = "Fail to parse protobuf in response at ";
    e.mutableMessage().append(filename);
    e.mutableMessage().push_back(':');
    pp::prettyPrint(e.mutableMessage(), line);
    return std::optional<OTSError>(std::move(e));
}

} // namespace

#define ISSUE()                                 \
    return issueError(__FILE__, __LINE__)

namespace impl {

template<class T>
std::optional<OTSError> readUint(T& out, uint8_t const *& b, uint8_t const * e)
{
    BOOST_STATIC_ASSERT(std::is_unsigned<T>::value);

    if (b + sizeof(T) > e) {
        ISSUE();
    }

    out = 0;
    for(size_t i = 0; i < sizeof(T); ++i) {
        T x = *b;
        x <<= 8 * i;
        out |= x;
        ++b;
    }

    return std::optional<OTSError>();
}

std::optional<OTSError> readUint64(uint64_t& out, uint8_t const *& b, uint8_t const * e)
{
    return readUint<uint64_t>(out, b, e);
}

std::optional<OTSError> readUint32(uint32_t& out, uint8_t const *& b, uint8_t const * e)
{
    return readUint<uint32_t>(out, b, e);
}

std::optional<OTSError> readUint8(uint8_t& out, uint8_t const *& b, uint8_t const * e)
{
    if (b + sizeof(uint8_t) > e) {
        ISSUE();
    }
    out = *b;
    ++b;
    return std::optional<OTSError>();
}

std::optional<OTSError> readHeader(uint8_t const *& b, uint8_t const * e)
{
    uint32_t header = 0;
    TRY(readUint32(header, b, e));
    if (header != kHeader) {
        ISSUE();
    }
    return std::optional<OTSError>();
}

std::optional<OTSError> readTag(Tag& out, uint8_t const *& b, uint8_t const * e)
{
    uint8_t res = 0;
    TRY(readUint8(res, b, e));
    out = static_cast<Tag>(res);
    return std::optional<OTSError>();
}

bool peekAndCheckTag(const Tag expect, uint8_t const * b, uint8_t const * e)
{
    if (b + sizeof(uint8_t) > e) {
        return false;
    }
    return static_cast<uint8_t>(expect) == *b;
}

std::optional<OTSError> readBlob(
    string& out,
    uint8_t& checksum,
    uint8_t const *& b,
    uint8_t const * e)
{
    uint32_t len = 0;
    TRY(readUint32(len, b, e));

    if (b + len > e) {
        ISSUE();
    }

    MemPiece p(b, len);
    out = p.to<string>();
    crc8U32(checksum, len);
    crc8MemPiece(checksum, p);
    b += len;

    return std::optional<OTSError>();
}

std::optional<OTSError> readName(
    string& out,
    uint8_t& checksum,
    uint8_t const *& b,
    uint8_t const * e)
{
    {
        Tag tag = kTag_None;
        TRY(readTag(tag, b, e));
        if (tag != kTag_CellName) {
            ISSUE();
        }
    }
    uint32_t len = 0;
    TRY(readUint32(len, b, e));

    if (b + len > e) {
        ISSUE();
    }

    MemPiece p(b, len);
    out = p.to<string>();
    crc8MemPiece(checksum, p);
    b += len;

    return std::optional<OTSError>();
}

std::optional<OTSError> readVariantType(
    VariantType& out,
    uint8_t const *& b,
    uint8_t const * e)
{
    deque<VariantType> types;
    types.push_back(kVT_Integer);
    types.push_back(kVT_Double);
    types.push_back(kVT_Boolean);
    types.push_back(kVT_String);
    types.push_back(kVT_Blob);

    uint8_t v = 0;
    TRY(readUint8(v, b, e));

    FOREACH_ITER(i, types) {
        if (static_cast<uint8_t>(*i) == v) {
            out = *i;
            return std::optional<OTSError>();
        }
    }
    ISSUE();
}

std::optional<OTSError> readPrimaryKeyValue(
    PrimaryKeyValue& out,
    uint8_t& checksum,
    uint8_t const *& b,
    uint8_t const * e)
{
    {
        Tag tag = kTag_None;
        TRY(readTag(tag, b, e));
        if (tag != kTag_CellValue) {
            ISSUE();
        }
    }
    {
        uint32_t placeholder = 0;
        TRY(readUint32(placeholder, b, e));
    }

    VariantType vt = kVT_Null;
    TRY(readVariantType(vt, b, e));
    switch(vt) {
    case kVT_Integer: {
        uint64_t v = 0;
        TRY(readUint64(v, b, e));
        out.mutableInteger() = v;
        crc8(checksum, static_cast<uint8_t>(kVT_Integer));
        crc8U64(checksum, v);
        break;
    }
    case kVT_String: {
        crc8(checksum, static_cast<uint8_t>(kVT_String));
        string v;
        TRY(readBlob(v, checksum, b, e));
        out.mutableStr() = v;
        break;
    }
    case kVT_Blob: {
        crc8(checksum, static_cast<uint8_t>(kVT_Blob));
        string v;
        TRY(readBlob(v, checksum, b, e));
        out.mutableBlob() = v;
        break;
    }

    case kVT_Double: case kVT_Boolean: case kVT_Null:
    case kVT_InfMin: case kVT_InfMax: case kVT_AutoIncrement:
        ISSUE();
    }
    return std::optional<OTSError>();
}

std::optional<OTSError> readPrimaryKeyColumn(
    PrimaryKeyColumn& out,
    uint8_t& checksum,
    uint8_t const *& b,
    uint8_t const * e)
{
    {
        Tag tag = kTag_None;
        TRY(readTag(tag, b, e));
        if (tag != kTag_Cell) {
            ISSUE();
        }
    }
    uint8_t colChecksum = 0;
    TRY(readName(out.mutableName(), colChecksum, b, e));
    TRY(readPrimaryKeyValue(out.mutableValue(), colChecksum, b, e));

    {
        Tag tag = kTag_None;
        TRY(readTag(tag, b, e));
        if (tag != kTag_CellChecksum) {
            ISSUE();
        }
        uint8_t oracle = 0;
        TRY(readUint8(oracle, b, e));
        if (colChecksum != oracle) {
            ISSUE();
        }
    }
    crc8(checksum, colChecksum);

    return std::optional<OTSError>();
}

std::optional<OTSError> readRowKey(
    PrimaryKey& out,
    uint8_t& checksum,
    uint8_t const *& b,
    uint8_t const * e)
{
    {
        Tag tag = kTag_None;
        TRY(readTag(tag, b, e));
        if (tag != kTag_RowKey) {
            ISSUE();
        }
    }
    for(;peekAndCheckTag(kTag_Cell, b, e);) {
        TRY(readPrimaryKeyColumn(out.append(), checksum, b, e));
    }
    return std::optional<OTSError>();
}

std::optional<OTSError> readAttrValue(
    AttributeValue& out,
    uint8_t& checksum,
    uint8_t const *& b,
    uint8_t const * e)
{
    {
        Tag tag = kTag_None;
        TRY(readTag(tag, b, e));
        if (tag != kTag_CellValue) {
            ISSUE();
        }
    }
    {
        uint32_t placeholder = 0;
        TRY(readUint32(placeholder, b, e));
    }

    VariantType vt = kVT_Null;
    TRY(readVariantType(vt, b, e));
    switch(vt) {
    case kVT_Integer: {
        uint64_t v = 0;
        TRY(readUint64(v, b, e));
        out.mutableInteger() = v;
        crc8(checksum, static_cast<uint8_t>(kVT_Integer));
        crc8U64(checksum, v);
        break;
    }
    case kVT_String: {
        crc8(checksum, static_cast<uint8_t>(kVT_String));
        string v;
        TRY(readBlob(v, checksum, b, e));
        out.mutableStr() = v;
        break;
    }
    case kVT_Blob: {
        crc8(checksum, static_cast<uint8_t>(kVT_Blob));
        string v;
        TRY(readBlob(v, checksum, b, e));
        out.mutableBlob() = v;
        break;
    }
    case kVT_Double: {
        uint64_t iv = 0;
        TRY(readUint64(iv, b, e));
        double v = 0;
        memcpy(static_cast<void*>(&v), static_cast<const void*>(&iv), sizeof(double));
        out.mutableFloatPoint() = v;
        crc8(checksum, static_cast<uint8_t>(kVT_Double));
        crc8U64(checksum, iv);
        break;
    }
    case kVT_Boolean: {
        uint8_t v = 0;
        TRY(readUint8(v, b, e));
        out.mutableBoolean() = v ? true : false;
        crc8(checksum, static_cast<uint8_t>(kVT_Boolean));
        crc8(checksum, static_cast<uint8_t>(out.boolean() ? 1 : 0));
        break;
    }

    case kVT_Null: case kVT_InfMin: case kVT_InfMax: case kVT_AutoIncrement:
        ISSUE();
    }
    return std::optional<OTSError>();
}

std::optional<OTSError> readstd::optionalCellTimestamp(
    std::optional<UtcTime>& out,
    uint8_t& checksum,
    uint8_t const *& b,
    uint8_t const * e)
{
    if (peekAndCheckTag(kTag_CellTimestamp, b, e)) {
        Tag tag = kTag_None;
        TRY(readTag(tag, b, e));
        uint64_t v = 0;
        TRY(readUint64(v, b, e));
        out.reset(UtcTime::fromMsec(static_cast<int64_t>(v)));
        crc8U64(checksum, v);
    }
    return std::optional<OTSError>();
}

std::optional<OTSError> readAttr(
    Attribute& out,
    uint8_t& checksum,
    uint8_t const *& b,
    uint8_t const * e)
{
    {
        Tag tag = kTag_None;
        TRY(readTag(tag, b, e));
        if (tag != kTag_Cell) {
            ISSUE();
        }
    }
    uint8_t colChecksum = 0;
    TRY(readName(out.mutableName(), colChecksum, b, e));
    TRY(readAttrValue(out.mutableValue(), colChecksum, b, e));
    TRY(readstd::optionalCellTimestamp(out.mutableTimestamp(), colChecksum, b, e));

    {
        Tag tag = kTag_None;
        TRY(readTag(tag, b, e));
        if (tag != kTag_CellChecksum) {
            ISSUE();
        }
        uint8_t oracle = 0;
        TRY(readUint8(oracle, b, e));
        if (colChecksum != oracle) {
            ISSUE();
        }
    }
    crc8(checksum, colChecksum);

    return std::optional<OTSError>();
}

std::optional<OTSError> readAttrs(
    IVector<Attribute>& out,
    uint8_t& checksum,
    uint8_t const *& b,
    uint8_t const * e)
{
    {
        Tag tag = kTag_None;
        TRY(readTag(tag, b, e));
        if (tag != kTag_RowData) {
            ISSUE();
        }
    }
    for(;peekAndCheckTag(kTag_Cell, b, e);) {
        TRY(readAttr(out.append(), checksum, b, e));
    }
    return std::optional<OTSError>();
}

std::optional<OTSError> readRow(Row& out, uint8_t const *& b, uint8_t const * e)
{
    uint8_t checksum = 0;

    TRY(readRowKey(out.mutablePrimaryKey(), checksum, b, e));

    if (peekAndCheckTag(kTag_RowData, b, e)) {
        TRY(readAttrs(out.mutableAttributes(), checksum, b, e));
    }

    crc8(checksum, 0); // for row-delete marker

    {
        Tag tag;
        TRY(readTag(tag, b, e));
        if (tag != kTag_RowChecksum) {
            ISSUE();
        }
        uint8_t oracle = 0;
        TRY(readUint8(oracle, b, e));
        if (checksum != oracle) {
            ISSUE();
        }
    }
    return std::optional<OTSError>();
}

std::optional<OTSError> readRows(IVector<Row>& out, uint8_t const *& b, uint8_t const * e)
{
    for(; b < e;) {
        Row row;
        TRY(impl::readRow(row, b, e));
        moveAssign(out.append(), std::move(row));
    }
    return std::optional<OTSError>();
}
} //namespace impl

std::optional<OTSError> readRow(Row& out, const MemPiece& p)
{
    const uint8_t* b = p.data();
    const uint8_t* e = b + p.length();
    TRY(impl::readHeader(b, e));
    TRY(impl::readRow(out, b, e));
    if (b != e) {
        ISSUE();
    }
    return std::optional<OTSError>();
}

std::optional<OTSError> readRows(IVector<Row>& out, const MemPiece& p)
{
    const uint8_t* b = p.data();
    const uint8_t* e = b + p.length();
    TRY(impl::readHeader(b, e));
    TRY(impl::readRows(out, b, e));
    if (b != e) {
        ISSUE();
    }
    return std::optional<OTSError>();
}

} // namespace plainbuffer
} // namespace core
} // namespace tablestore
} // namespace aliyun
