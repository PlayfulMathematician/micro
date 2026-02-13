/*
png.cpp - A single file png writer
Copyright (C) 2026 Playful Mathematician <me@playfulmathematician>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published by
the Free Software Foundation, version 3.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <ostream>
#include <vector>
struct Image {
  uint32_t width;
  uint32_t height;
  std::vector<uint8_t> rgba;
};

static uint32_t crc_table[256];

static void crc32_init(void) {
  uint32_t crc32 = 1;
  for (unsigned int i = 128; i; i >>= 1) {
    crc32 = (crc32 >> 1) ^ (crc32 & 1 ? 0xedb88320 : 0);
    for (unsigned int j = 0; j < 256; j += 2 * i)
      crc_table[i + j] = crc32 ^ crc_table[j];
  }
}

uint32_t crc32(const std::vector<unsigned char> &v) {
  uint32_t crc32 = 0xFFFFFFFFu;
  if (crc_table[255] == 0)
    crc32_init();
  for (unsigned char item : v) {
    crc32 ^= item;
    crc32 = (crc32 >> 8) ^ crc_table[crc32 & 0xFF];
  }
  crc32 ^= 0xFFFFFFFFu;
  return crc32;
}
std::vector<unsigned char> u32_be(uint32_t value) {
  std::vector<unsigned char> bytes = {
      static_cast<unsigned char>((value >> 030) & 0xFF),
      static_cast<unsigned char>((value >> 020) & 0xFF),
      static_cast<unsigned char>((value >> 010) & 0xFF),
      static_cast<unsigned char>((value >> 000) & 0xFF)};
  return bytes;
}
void write_u32_be(uint32_t value, std::ostream &stream) {
  std::vector<unsigned char> bytes = u32_be(value);
  stream.write(reinterpret_cast<const char *>(bytes.data()), 4);
}
void write_to_file(const std::vector<unsigned char> &vec,
                   std::ostream &stream) {
  stream.write(reinterpret_cast<const char *>(vec.data()),
               static_cast<std::streamsize>(vec.size()));
}
void write_header(std::ostream &stream) {
  std::vector<unsigned char> header = {0x89, 0x50, 0x4e, 0x47,
                                       0x0d, 0x0a, 0x1a, 0x0a};
  write_to_file(header, stream);
}

void write_chunk(const std::vector<unsigned char> &vec,
                 const std::string &chunk_name, std::ostream &stream) {
  write_u32_be(vec.size(), stream);
  std::vector<unsigned char> buffer;
  for (char c : chunk_name) {
    buffer.push_back(c);
  }
  for (char c : vec) {
    buffer.push_back(c);
  }
  write_to_file(buffer, stream);
  uint32_t crc = crc32(buffer);
  write_u32_be(crc, stream);
}
void write_ihdr(const Image &img, std::ostream &stream) {
  std::vector<unsigned char> buffer;
  std::vector<unsigned char> width_buffer = u32_be(img.width);
  std::vector<unsigned char> height_buffer = u32_be(img.height);
  for (unsigned char c : width_buffer) {
    buffer.push_back(c);
  }
  for (unsigned char c : height_buffer) {
    buffer.push_back(c);
  }
  buffer.push_back(0x08);
  buffer.push_back(0x06);
  buffer.push_back(0x00);
  buffer.push_back(0x00);
  buffer.push_back(0x00);
  write_chunk(buffer, "IHDR", stream);
}

void write_iend(std::ostream &stream) {
  std::vector<unsigned char> buffer;
  write_chunk(buffer, "IEND", stream);
}

uint32_t adler32(const std::vector<unsigned char> &data) {
  uint32_t a = 1;
  uint32_t b = 0;

  for (unsigned char c : data) {
    a = (a + c) % 65521;
    b = (b + a) % 65521;
  }

  return (b << 16) | a;
}

void write_idat(const Image &img, std::ostream &stream) {
  std::vector<unsigned char> filtered;
  for (uint32_t i = 0; i < img.height; i++) {
    filtered.push_back(0x00);
    for (uint32_t j = 0; j < 4 * img.width; j++) {
      filtered.push_back(img.rgba[4 * img.width * i + j]);
    }
  }
  std::vector<unsigned char> buffer;
  buffer.push_back(0x78);
  buffer.push_back(0x01);
  size_t offset = 0;
  size_t remaining = filtered.size();

  while (remaining > 0) {
    uint16_t chunk_len =
        remaining > 65535 ? 65535 : static_cast<uint16_t>(remaining);

    bool final_block = (remaining <= 65535);
    buffer.push_back(final_block ? 0x01 : 0x00);

    uint16_t len = chunk_len;
    uint16_t nlen = ~len;
    buffer.push_back(len & 0xFF);
    buffer.push_back((len >> 8) & 0xFF);
    buffer.push_back(nlen & 0xFF);
    buffer.push_back((nlen >> 8) & 0xFF);
    buffer.insert(buffer.end(), filtered.begin() + offset,
                  filtered.begin() + offset + chunk_len);
    offset += chunk_len;
    remaining -= chunk_len;
  }
  uint32_t chk = adler32(filtered);
  for (char c : u32_be(chk)) {
    buffer.push_back(c);
  }
  write_chunk(buffer, "IDAT", stream);
}

void write_png(const Image &img, std::ostream &stream) {
  write_header(stream);
  write_ihdr(img, stream);
  write_idat(img, stream);
  write_iend(stream);
}

int main() {
  Image img;
  img.width = 1024;
  img.height = 1024;

  img.rgba = std::vector<uint8_t>();
  for (uint32_t y = 0; y < img.height; y++) {
    for (uint32_t x = 0; x < img.width; x++) {
      img.rgba.push_back(x / 4);
      img.rgba.push_back(y / 4);
      img.rgba.push_back(x / 8 + y / 8);
      img.rgba.push_back(255);
    }
  }
  std::ofstream file("image.png", std::ios::binary);
  write_png(img, file);
}
