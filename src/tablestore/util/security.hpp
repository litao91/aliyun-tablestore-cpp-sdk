#pragma once
#ifndef TABLESTORE_UTIL_SECURITY_HPP
#define TABLESTORE_UTIL_SECURITY_HPP
/*
BSD 3-Clause License

Copyright (c) 2017, Alibaba Cloud
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "tablestore/util/mempiece.hpp"
#include <deque>
#include <memory>
#include <stdint.h>
#include <string>

namespace aliyun {
namespace tablestore {
namespace util {

namespace impl {
class Md5Ctx;
} // namespace impl

class Md5 {
public:
  static const int64_t kLength = 16;

  explicit Md5();
  ~Md5();

  void update(const MemPiece &);
  void finalize(const MutableMemPiece &);

private:
  impl::Md5Ctx *mCtx;
  bool mFinalized;
};

std::string md5(const std::deque<util::MemPiece> &);

namespace impl {
class Sha1Ctx;
} // namespace impl

class Sha1 {
public:
  static const int64_t kLength = 20;

  explicit Sha1();
  ~Sha1();

  void update(const MemPiece &);
  void finalize(const MutableMemPiece &);

private:
  impl::Sha1Ctx *mCtx;
  bool mFinalized;
};

class HmacSha1 {
public:
  static const int64_t kLength = Sha1::kLength;

  explicit HmacSha1(const MemPiece &key);
  ~HmacSha1();

  void update(const MemPiece &);
  void finalize(const MutableMemPiece &);

private:
  Sha1 mPass1;
  Sha1 mPass2;
  bool mFinalized;
};

namespace impl {
class Base64Ctx;
} // namespace impl

class Base64Encoder {
public:
  explicit Base64Encoder();
  ~Base64Encoder();

  void update(const MemPiece &);
  void finalize();
  MemPiece base64() const;

private:
  impl::Base64Ctx *mCtx;
  bool mFinalized;
};

/**
 * crc-8-ATM
 */
inline void crc8(uint8_t &out, const uint8_t in) {
  static const uint8_t kTable[] = {
      0x00, 0x07, 0x0e, 0x09, 0x1c, 0x1b, 0x12, 0x15, 0x38, 0x3f, 0x36, 0x31,
      0x24, 0x23, 0x2a, 0x2d, 0x70, 0x77, 0x7e, 0x79, 0x6c, 0x6b, 0x62, 0x65,
      0x48, 0x4f, 0x46, 0x41, 0x54, 0x53, 0x5a, 0x5d, 0xe0, 0xe7, 0xee, 0xe9,
      0xfc, 0xfb, 0xf2, 0xf5, 0xd8, 0xdf, 0xd6, 0xd1, 0xc4, 0xc3, 0xca, 0xcd,
      0x90, 0x97, 0x9e, 0x99, 0x8c, 0x8b, 0x82, 0x85, 0xa8, 0xaf, 0xa6, 0xa1,
      0xb4, 0xb3, 0xba, 0xbd, 0xc7, 0xc0, 0xc9, 0xce, 0xdb, 0xdc, 0xd5, 0xd2,
      0xff, 0xf8, 0xf1, 0xf6, 0xe3, 0xe4, 0xed, 0xea, 0xb7, 0xb0, 0xb9, 0xbe,
      0xab, 0xac, 0xa5, 0xa2, 0x8f, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9d, 0x9a,
      0x27, 0x20, 0x29, 0x2e, 0x3b, 0x3c, 0x35, 0x32, 0x1f, 0x18, 0x11, 0x16,
      0x03, 0x04, 0x0d, 0x0a, 0x57, 0x50, 0x59, 0x5e, 0x4b, 0x4c, 0x45, 0x42,
      0x6f, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7d, 0x7a, 0x89, 0x8e, 0x87, 0x80,
      0x95, 0x92, 0x9b, 0x9c, 0xb1, 0xb6, 0xbf, 0xb8, 0xad, 0xaa, 0xa3, 0xa4,
      0xf9, 0xfe, 0xf7, 0xf0, 0xe5, 0xe2, 0xeb, 0xec, 0xc1, 0xc6, 0xcf, 0xc8,
      0xdd, 0xda, 0xd3, 0xd4, 0x69, 0x6e, 0x67, 0x60, 0x75, 0x72, 0x7b, 0x7c,
      0x51, 0x56, 0x5f, 0x58, 0x4d, 0x4a, 0x43, 0x44, 0x19, 0x1e, 0x17, 0x10,
      0x05, 0x02, 0x0b, 0x0c, 0x21, 0x26, 0x2f, 0x28, 0x3d, 0x3a, 0x33, 0x34,
      0x4e, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5c, 0x5b, 0x76, 0x71, 0x78, 0x7f,
      0x6a, 0x6d, 0x64, 0x63, 0x3e, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2c, 0x2b,
      0x06, 0x01, 0x08, 0x0f, 0x1a, 0x1d, 0x14, 0x13, 0xae, 0xa9, 0xa0, 0xa7,
      0xb2, 0xb5, 0xbc, 0xbb, 0x96, 0x91, 0x98, 0x9f, 0x8a, 0x8d, 0x84, 0x83,
      0xde, 0xd9, 0xd0, 0xd7, 0xc2, 0xc5, 0xcc, 0xcb, 0xe6, 0xe1, 0xe8, 0xef,
      0xfa, 0xfd, 0xf4, 0xf3};
  out = kTable[out ^ in];
}

inline void crc8U32(uint8_t &out, uint32_t in) {
  crc8(out, in & 0xFF);
  crc8(out, (in >> 8) & 0xFF);
  crc8(out, (in >> 16) & 0xFF);
  crc8(out, in >> 24);
}

inline void crc8U64(uint8_t &out, uint64_t in) {
  for (int64_t i = 0, sz = sizeof(in); i < sz; ++i) {
    crc8(out, in & 0xFF);
    in >>= 8;
  }
}

inline void crc8MemPiece(uint8_t &out, const MemPiece &in) {
  if (in.length() == 0) {
    return;
  }
  const uint8_t *b = in.data();
  const uint8_t *e = b + in.length();
  for (; b < e; ++b) {
    crc8(out, *b);
  }
}

class Adler32 {
  static const uint32_t kMod = 65521; // the largest prime number below 2^16
public:
  explicit Adler32() : mA(1), mB(0) {}

  explicit Adler32(uint32_t x) {
    mA = x & 0xFFFF;
    mB = x >> 16;
  }

  void update(uint8_t x) {
    mA = (mA + x) % kMod;
    mB = (mB + mA) % kMod;
  }

  uint32_t get() const { return (mB << 16) | mA; }

private:
  uint32_t mA;
  uint32_t mB;
};

namespace impl {

template <class T, class Enable = void> struct Adler32Updater {};

template <> struct Adler32Updater<MemPiece, void> {
  void operator()(Adler32 &adl, const MemPiece &in) const {
    const uint8_t *b = in.data();
    const uint8_t *e = b + in.length();
    for (; b < e; ++b) {
      adl.update(*b);
    }
  }
};

} // namespace impl

template <class T> void update(Adler32 &adl, const T &x) {
  impl::Adler32Updater<T> updater;
  updater(adl, x);
}

} // namespace util
} // namespace tablestore
} // namespace aliyun
#endif
