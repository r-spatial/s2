/**********************************************************************
 *
 * {wk} for R borrowed this code from...
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: io/WKTReader.java rev. 1.1 (JTS-1.7)
 * Modifications for {wk}: make self-contained, don't use namespace std,
 *   modify for event-based parse style.
 *
 **********************************************************************/

#ifndef WK_WKT_STREAMER_H
#define WK_WKT_STREAMER_H

#include <string>
#include <clocale>

#include "wk/reader.h"
#include "wk/io-string.h"
#include "wk/error-formatter.h"
#include "wk/geometry-handler.h"
#include "wk/string-tokenizer.h"
#include "wk/parse-exception.h"
#include "wk/coord.h"


class WKTStreamer: public WKReader {
public:

  WKTStreamer(WKStringProvider& provider): WKReader(provider), provider(provider) {
    // TODO evaluate if we need this if we use C++11's double parser
#ifdef _MSC_VER
    // Avoid multithreading issues caused by setlocale
    _configthreadlocale(_ENABLE_PER_THREAD_LOCALE);
#endif
    char* p = std::setlocale(LC_NUMERIC, nullptr);
    if(nullptr != p) {
      this->saved_locale = p;
    }
    std::setlocale(LC_NUMERIC, "C");
  }

  ~WKTStreamer() {
    std::setlocale(LC_NUMERIC, saved_locale.c_str());
  }

  void readFeature(size_t featureId) {
    this->handler->nextFeatureStart(featureId);

    if (this->provider.featureIsNull()) {
      this->handler->nextNull(featureId);
    } else {
      const std::string& wellKnownText = this->provider.featureString();
      WKStringTokenizer tokenizer(wellKnownText);
      this->readGeometryTaggedText(&tokenizer, PART_ID_NONE, SRID_UNKNOWN);
    }

    this->handler->nextFeatureEnd(featureId);
  }

protected:
  WKStringProvider& provider;

  void readGeometryTaggedText(WKStringTokenizer* tokenizer, uint32_t partId, uint32_t srid) {
    std::string type = this->getNextWord(tokenizer);
    int geometryType;

    if (type == "SRID" && srid == SRID_UNKNOWN) {
      this->readGeometryTaggedText(tokenizer, partId, this->getSRID(tokenizer));
      return;
    } else if(type == "POINT") {
      geometryType = WKGeometryType::Point;

    } else if(type == "LINESTRING") {
      geometryType = WKGeometryType::LineString;

    } else if(type == "POLYGON") {
      geometryType = WKGeometryType::Polygon;

    } else if(type == "MULTIPOINT") {
      geometryType = WKGeometryType::MultiPoint;

    } else if(type == "MULTILINESTRING") {
      geometryType = WKGeometryType::MultiLineString;

    } else if(type == "MULTIPOLYGON") {
      geometryType = WKGeometryType::MultiPolygon;

    } else if(type == "GEOMETRYCOLLECTION") {
      geometryType = WKGeometryType::GeometryCollection;

    } else {
      throw WKParseException(ErrorFormatter() << "Unknown type " << type);
    }

    WKGeometryMeta meta = this->getNextEmptyOrOpener(tokenizer, geometryType);
    if (srid != SRID_UNKNOWN) {
      meta.hasSRID = true;
      meta.srid = srid;
    }

    this->readGeometry(tokenizer, meta, partId);
  }

  void readGeometry(WKStringTokenizer* tokenizer, const WKGeometryMeta meta, uint32_t partId) {
    this->handler->nextGeometryStart(meta, partId);

    // if empty, calling read* functions will fail because
    // the empty token has been consumed
    if (meta.size == 0) {
      this->handler->nextGeometryEnd(meta, partId);
      return;
    }

    switch (meta.geometryType) {

    case WKGeometryType::Point:
      this->readPointText(tokenizer, meta);
      break;

    case WKGeometryType::LineString:
      this->readLineStringText(tokenizer, meta);
      break;

    case WKGeometryType::Polygon:
      this->readPolygonText(tokenizer, meta);
      break;

    case WKGeometryType::MultiPoint:
      this->readMultiPointText(tokenizer, meta);
      break;

    case WKGeometryType::MultiLineString:
      this->readMultiLineStringText(tokenizer, meta);
      break;

    case WKGeometryType::MultiPolygon:
      this->readMultiPolygonText(tokenizer, meta);
      break;

    case WKGeometryType::GeometryCollection:
      this->readGeometryCollectionText(tokenizer, meta);
      break;

    default:
      throw WKParseException(
          ErrorFormatter() <<
            "Unrecognized geometry type: " <<
            meta.geometryType
      );
    }

    this->handler->nextGeometryEnd(meta, partId);
  }

  void readPointText(WKStringTokenizer* tokenizer, const WKGeometryMeta meta) {
    this->readCoordinate(tokenizer, meta, 0);
    this->getNextCloser(tokenizer);
  }

  void readLineStringText(WKStringTokenizer* tokenizer, const WKGeometryMeta meta) {
    this->readCoordinates(tokenizer, meta);
  }

  void readPolygonText(WKStringTokenizer* tokenizer, const WKGeometryMeta meta) {
    uint32_t ringId = 0;
    std::string nextToken;
    do {
      this->readLinearRingText(tokenizer, meta, ringId);
      ringId++;
      nextToken = this->getNextCloserOrComma(tokenizer);
    } while (nextToken == ",");
  }

  void readLinearRingText(WKStringTokenizer* tokenizer, const WKGeometryMeta meta, uint32_t ringId) {
    this->handler->nextLinearRingStart(meta, WKGeometryMeta::SIZE_UNKNOWN, ringId);
    this->getNextEmptyOrOpener(tokenizer, 0);
    this->readCoordinates(tokenizer, meta);
    this->handler->nextLinearRingEnd(meta, WKGeometryMeta::SIZE_UNKNOWN, ringId);
  }

  void readMultiPointText(WKStringTokenizer* tokenizer, const WKGeometryMeta meta) {

    int tok = tokenizer->peekNextToken();

    // the next token can be EMPTY, which we only
    // deal with as the 'correct' multipoint form
    std::string nextWord;
    if (tok == WKStringTokenizer::TT_WORD) {
      nextWord = tokenizer->getSVal();
    }

    if (tok == WKStringTokenizer::TT_NUMBER) {
      // Try to parse deprecated form "MULTIPOINT(0 0, 1 1)"
      std::string nextToken;
      uint32_t partId = 0;
      do {
        WKGeometryMeta childMeta(WKGeometryType::Point, meta.hasZ, meta.hasM, meta.hasSRID);
        childMeta.srid = meta.srid;

        this->handler->nextGeometryStart(childMeta, partId);
        this->readCoordinate(tokenizer, meta, 0);
        this->handler->nextGeometryEnd(childMeta, partId);
        partId++;

        nextToken = this->getNextCloserOrComma(tokenizer);
      } while (nextToken == ",");

    } else if(tok == '(' || (tok == WKStringTokenizer::TT_WORD && nextWord == "EMPTY")) {
      // Try to parse correct form "MULTIPOINT((0 0), (1 1))"
      std::string nextToken;
      uint32_t partId = 0;
      do {
        WKGeometryMeta childMeta = this->getNextEmptyOrOpener(tokenizer, WKGeometryType::Point);
        childMeta.hasZ = meta.hasZ;
        childMeta.hasM = meta.hasM;
        childMeta.srid = meta.srid;

        this->readGeometry(tokenizer, childMeta, partId);
        partId++;

        nextToken = getNextCloserOrComma(tokenizer);
      } while (nextToken == ",");

    } else {
      std::stringstream err;
      err << "Unexpected token: ";
      switch(tok) {
      case WKStringTokenizer::TT_WORD:
        err << "WORD " << nextWord;
        break;
      case WKStringTokenizer::TT_NUMBER:
        err << "NUMBER " << tokenizer->getNVal();
        break;
      case WKStringTokenizer::TT_EOF:
      case WKStringTokenizer::TT_EOL:
        err << "EOF or EOL";
        break;
      case '(':
        err << "(";
        break;
      case ')':
        err << ")";
        break;
      case ',':
        err << ",";
        break;
      default:
        err << "??";
      break;
      }

      err << std::endl;
      throw WKParseException(err.str());
    }
  }

  void readMultiLineStringText(WKStringTokenizer* tokenizer, const WKGeometryMeta meta) {
    std::string nextToken;
    uint32_t partId = 0;
    do {
      WKGeometryMeta childMeta = this->getNextEmptyOrOpener(tokenizer, WKGeometryType::LineString);
      childMeta.hasZ = meta.hasZ;
      childMeta.hasM = meta.hasM;
      childMeta.srid = meta.srid;

      this->readGeometry(tokenizer, childMeta, partId);
      partId++;

      nextToken = this->getNextCloserOrComma(tokenizer);
    } while (nextToken == ",");
  }

  void readMultiPolygonText(WKStringTokenizer* tokenizer, const WKGeometryMeta meta) {
    std::string nextToken;
    uint32_t partId = 0;
    do {
      WKGeometryMeta childMeta = this->getNextEmptyOrOpener(tokenizer, WKGeometryType::Polygon);
      childMeta.hasZ = meta.hasZ;
      childMeta.hasM = meta.hasM;
      childMeta.srid = meta.srid;

      this->readGeometry(tokenizer, childMeta, partId);
      partId++;

      nextToken = this->getNextCloserOrComma(tokenizer);
    } while (nextToken == ",");
  }

  void readGeometryCollectionText(WKStringTokenizer* tokenizer, const WKGeometryMeta meta) {
    std::string nextToken;

    uint32_t partId = 0;
    do {
      this->readGeometryTaggedText(tokenizer, partId, SRID_UNKNOWN);
      partId++;

      nextToken = this->getNextCloserOrComma(tokenizer);
    } while (nextToken == ",");
  }

  void readCoordinates(WKStringTokenizer* tokenizer, const WKGeometryMeta meta) {
    uint32_t coordId = 0;
    std::string nextToken;
    do {
      this->readCoordinate(tokenizer, meta, coordId);
      coordId++;
      nextToken = this->getNextCloserOrComma(tokenizer);
    } while (nextToken == ",");
  }

  void readCoordinate(WKStringTokenizer* tokenizer, const WKGeometryMeta meta, uint32_t coordId) {
    WKCoord coord;
    coord.x = this->getNextNumber(tokenizer);
    coord.y = this->getNextNumber(tokenizer);

    if (this->isNumberNext(tokenizer)) {
      if (meta.hasZ) {
        coord.z = this->getNextNumber(tokenizer);
        coord.hasZ = true;
      } else if (meta.hasM) {
        coord.m = this->getNextNumber(tokenizer);
        coord.hasM = true;
      } else {
        throw WKParseException(ErrorFormatter() << "Found unexpected coordiate " << this->getNextNumber(tokenizer));
      }

      if(this->isNumberNext(tokenizer)) {
        if (meta.hasM) {
          coord.m = this->getNextNumber(tokenizer);
          coord.hasM = true;
        } else {
          throw WKParseException(ErrorFormatter() << "Found unexpected coordiate " << this->getNextNumber(tokenizer));
        }
      } else if (meta.hasZ && meta.hasM) {
        throw WKParseException("Expected M coordinate but foound ','  or ')'");
      }
    } else if (meta.hasZ || meta.hasM) {
      throw WKParseException("Expected Z or M coordinate");
    }

    this->handler->nextCoordinate(meta, coord, coordId);
  }

  double getNextNumber(WKStringTokenizer* tokenizer) {
    int type = tokenizer->nextToken();
    switch(type) {
    case WKStringTokenizer::TT_NUMBER:
      return tokenizer->getNVal();
    case WKStringTokenizer::TT_EOF:
      throw WKParseException("Expected number but encountered end of stream");
    case WKStringTokenizer::TT_EOL:
      throw WKParseException("Expected number but encountered end of line");
    case WKStringTokenizer::TT_WORD:
      throw WKParseException(ErrorFormatter() << "Expected number but encountered word " << tokenizer->getSVal());
    case '(':
      throw WKParseException("Expected number but encountered '('");
    case ')':
      throw WKParseException("Expected number but encountered ')'");
    case ',':
      throw WKParseException("Expected number but encountered ','");
    default:
      throw std::runtime_error(ErrorFormatter() << "getNextNumber(): Unexpected token type " << type);
    }
  }

  int getSRID(WKStringTokenizer* tokenizer) {
    int nextToken = tokenizer->peekNextToken();
    if (nextToken == '=') {
      // consume the "="
      this->getNextWord(tokenizer);
      int srid = this->getNextNumber(tokenizer);
      std::string nextWord = this->getNextWord(tokenizer);
      if (nextWord == ";") {
        return srid;
      } else {
        throw WKParseException(ErrorFormatter() << "Expected ';' but found " << nextWord);
      }
    } else {
      // better error message here will require some refactoring of all the
      // errors
      throw WKParseException(ErrorFormatter() <<  "Expected '=' and SRID");
    }
  }

  WKGeometryMeta getNextEmptyOrOpener(WKStringTokenizer* tokenizer, int geometryType) {
    std::string nextWord = this->getNextWord(tokenizer);

    bool hasZ = false;
    bool hasM = false;
    bool hasSRID = false;

    // Parse the Z, M or ZM of an SF1.2 3/4 dim coordinate.
    if (nextWord == "Z") {
      hasZ = true;
      nextWord = this->getNextWord(tokenizer);
    } else if (nextWord == "M") {
      hasM = true;
      nextWord = this->getNextWord(tokenizer);
    } else if (nextWord == "ZM") {
      hasZ = true;
      hasM = true;
      nextWord = this->getNextWord(tokenizer);
    }

    WKGeometryMeta meta = WKGeometryMeta(geometryType, hasZ, hasM, hasSRID);
    if(nextWord == "EMPTY") {
      meta.hasSize = true;
      meta.size = 0;
      return meta;

    } else if (nextWord == "(") {
      return meta;

    } else {
      throw WKParseException(
          ErrorFormatter() <<
            "Expected 'Z', 'M', 'ZM', 'EMPTY' or '(' but encountered " <<
              nextWord
      );
    }
  }

  std::string getNextCloserOrComma(WKStringTokenizer* tokenizer) {
    std::string nextWord = this->getNextWord(tokenizer);
    if(nextWord == "," || nextWord == ")") {
      return nextWord;
    }

    throw  WKParseException(ErrorFormatter() << "Expected ')' or ',' but encountered" << nextWord);
  }

  std::string getNextCloser(WKStringTokenizer* tokenizer) {
    std::string nextWord = getNextWord(tokenizer);
    if(nextWord == ")") {
      return nextWord;
    }

    throw WKParseException(ErrorFormatter() << "Expected ')' but encountered" << nextWord);
  }

  std::string getNextWord(WKStringTokenizer* tokenizer) {
    int type = tokenizer->nextToken();
    switch(type) {
    case WKStringTokenizer::TT_WORD: {
      std::string word = tokenizer->getSVal();
      int i = static_cast<int>(word.size());
      while(--i >= 0) {
        word[i] = static_cast<char>(toupper(word[i]));
      }
      return word;
    }
    case WKStringTokenizer::TT_EOF:
      throw WKParseException("Expected word but encountered end of stream");
    case WKStringTokenizer::TT_EOL:
      throw WKParseException("Expected word but encountered end of line");
    case WKStringTokenizer::TT_NUMBER:
      throw WKParseException(ErrorFormatter() << "Expected word but encountered number" << tokenizer->getNVal());
    case '(':
      return "(";
    case ')':
      return ")";
    case ',':
      return ",";
    case ';':
      return ";";
    case '=':
      return "=";
    default:
      throw std::runtime_error(ErrorFormatter() << "Unexpected token type " << type);
    }
  }

  bool isNumberNext(WKStringTokenizer* tokenizer) {
    return tokenizer->peekNextToken() == WKStringTokenizer::TT_NUMBER;
  }

private:
  std::string saved_locale;
  const static uint32_t SRID_UNKNOWN = UINT32_MAX;

};

#endif
