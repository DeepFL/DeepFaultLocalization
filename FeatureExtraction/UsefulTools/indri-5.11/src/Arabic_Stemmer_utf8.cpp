/*==========================================================================
 * Copyright (c) 2012 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
*/
#include "indri/Arabic_Stemmer_utf8.hpp"
#include "lemur/Exception.hpp"
#include <iostream>
#include <cstring>
#include <cstdlib>


#define CHAR_WAW  0x0648		// Unicode code point for WAW
#define CHAR_NULL 0x0000		// Unicode code point for null

// Unicode arabic range
#define ARABIC_RANGE_LOW    0x0600
#define ARABIC_RANGE_HIGH   0x06FF

#define NUMSTEMMERS 6
namespace indri
{
  namespace parse 
  {

  const UINT64 Arabic_Stemmer_utf8::defArticle0[] = {0x0627,0x0644,NULL };
  const UINT64 Arabic_Stemmer_utf8::defArticle1[] = {0x0648,0x0627,0x0644,NULL };
  const UINT64 Arabic_Stemmer_utf8::defArticle2[] = {0x0628,0x0627,0x0644,NULL };
  const UINT64 Arabic_Stemmer_utf8::defArticle3[] = {0x0643,0x0627,0x0644,NULL };
  const UINT64 Arabic_Stemmer_utf8::defArticle4[] = {0x0641,0x0627,0x0644,NULL };
  const UINT64 Arabic_Stemmer_utf8::defArticle5[] = {0x0644,0x0644,NULL };
  const UINT64 *Arabic_Stemmer_utf8::allDefArticles[] = {defArticle0,defArticle1,defArticle2,defArticle3,defArticle4,defArticle5,NULL};

  const UINT64 Arabic_Stemmer_utf8::suffix0[] = {0x0647,0x0627,NULL };
  const UINT64 Arabic_Stemmer_utf8::suffix1[] = {0x0627,0x0646,NULL };
  const UINT64 Arabic_Stemmer_utf8::suffix2[] = {0x0627,0x062A,NULL };
  const UINT64 Arabic_Stemmer_utf8::suffix3[] = {0x0648,0x0646,NULL };
  const UINT64 Arabic_Stemmer_utf8::suffix4[] = {0x064A,0x0646,NULL };
  const UINT64 Arabic_Stemmer_utf8::suffix5[] = {0x064A,0x0647,NULL };
  const UINT64 Arabic_Stemmer_utf8::suffix6[] = {0x064A,0x0629,NULL };
  const UINT64 Arabic_Stemmer_utf8::suffix7[] = {0x0647,NULL };
  const UINT64 Arabic_Stemmer_utf8::suffix8[] = {0x0629,NULL };
  const UINT64 Arabic_Stemmer_utf8::suffix9[] = {0x064A,NULL };
  const UINT64 *Arabic_Stemmer_utf8::allSuffixes[] = {suffix0,suffix1,suffix2,suffix3,suffix4,suffix5,suffix6,suffix7,suffix8,suffix9,NULL};

  const UINT64 Arabic_Stemmer_utf8::stopWord0[] = {0x0627,0x0646,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord1[] = {0x0628,0x0639,0x062F,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord2[] = {0x0636,0x062F,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord3[] = {0x064A,0x0644,0x064A,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord4[] = {0x0627,0x0644,0x0649,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord5[] = {0x0641,0x064A,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord6[] = {0x0645,0x0646,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord7[] = {0x062D,0x062A,0x0649,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord8[] = {0x0648,0x0647,0x0648,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord9[] = {0x064A,0x0643,0x0648,0x0646,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord10[] = {0x0628,0x0647,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord11[] = {0x0648,0x0644,0x064A,0x0633,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord12[] = {0x0623,0x062D,0x062F,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord13[] = {0x0639,0x0644,0x0649,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord14[] = {0x0648,0x0643,0x0627,0x0646,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord15[] = {0x062A,0x0644,0x0643,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord16[] = {0x0643,0x0630,0x0644,0x0643,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord17[] = {0x0627,0x0644,0x062A,0x064A,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord18[] = {0x0648,0x0628,0x064A,0x0646,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord19[] = {0x0641,0x064A,0x0647,0x0627,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord20[] = {0x0639,0x0644,0x064A,0x0647,0x0627,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord21[] = {0x0625,0x0646,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord22[] = {0x0648,0x0639,0x0644,0x0649,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord23[] = {0x0644,0x0643,0x0646,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord24[] = {0x0639,0x0646,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord25[] = {0x0645,0x0633,0x0627,0x0621,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord26[] = {0x0644,0x064A,0x0633,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord27[] = {0x0645,0x0646,0x0630,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord28[] = {0x0627,0x0644,0x0630,0x064A,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord29[] = {0x0623,0x0645,0x0627,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord30[] = {0x062D,0x064A,0x0646,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord31[] = {0x0648,0x0645,0x0646,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord32[] = {0x0644,0x0627,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord33[] = {0x0644,0x064A,0x0633,0x0628,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord34[] = {0x0648,0x0643,0x0627,0x0646,0x062A,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord35[] = {0x0623,0x064A,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord36[] = {0x0645,0x0627,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord37[] = {0x0639,0x0646,0x0647,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord38[] = {0x062D,0x0648,0x0644,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord39[] = {0x062F,0x0648,0x0646,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord40[] = {0x0645,0x0639,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord41[] = {0x0644,0x0643,0x0646,0x0647,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord42[] = {0x0648,0x0644,0x0643,0x0646,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord43[] = {0x0644,0x0647,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord44[] = {0x0647,0x0630,0x0627,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord45[] = {0x0648,0x0627,0x0644,0x062A,0x064A,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord46[] = {0x0641,0x0642,0x0637,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord47[] = {0x062B,0x0645,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord48[] = {0x0647,0x0630,0x0647,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord49[] = {0x0623,0x0646,0x0647,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord50[] = {0x062A,0x0643,0x0648,0x0646,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord51[] = {0x0642,0x062F,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord52[] = {0x0628,0x064A,0x0646,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord53[] = {0x062C,0x062F,0x0627,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord54[] = {0x0644,0x0646,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord55[] = {0x0646,0x062D,0x0648,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord56[] = {0x0643,0x0627,0x0646,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord57[] = {0x0644,0x0647,0x0645,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord58[] = {0x0644,0x0623,0x0646,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord59[] = {0x0627,0x0644,0x064A,0x0648,0x0645,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord60[] = {0x0644,0x0645,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord61[] = {0x0647,0x0624,0x0644,0x0627,0x0621,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord62[] = {0x0641,0x0625,0x0646,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord63[] = {0x0641,0x064A,0x0647,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord64[] = {0x0630,0x0644,0x0643,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord65[] = {0x0644,0x0648,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord66[] = {0x0639,0x0646,0x062F,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord67[] = {0x0627,0x0644,0x0644,0x0630,0x064A,0x0646,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord68[] = {0x0643,0x0644,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord69[] = {0x0628,0x062F,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord70[] = {0x0644,0x062F,0x0649,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord71[] = {0x0648,0x062B,0x064A,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord72[] = {0x0623,0x0646,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord73[] = {0x0648,0x0645,0x0639,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord74[] = {0x0641,0x0642,0x062F,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord75[] = {0x0628,0x0644,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord76[] = {0x0647,0x0648,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord77[] = {0x0639,0x0646,0x0647,0x0627,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord78[] = {0x0645,0x0646,0x0647,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord79[] = {0x0628,0x0647,0x0627,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord80[] = {0x0648,0x0641,0x064A,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord81[] = {0x0641,0x0647,0x0648,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord82[] = {0x062A,0x062D,0x062A,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord83[] = {0x0644,0x0647,0x0627,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord84[] = {0x0623,0x0648,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord85[] = {0x0625,0x0630,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord86[] = {0x0639,0x0644,0x064A,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord87[] = {0x0639,0x0644,0x064A,0x0647,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord88[] = {0x0643,0x0645,0x0627,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord89[] = {0x0643,0x064A,0x0641,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord90[] = {0x0647,0x0646,0x0627,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord91[] = {0x0648,0x0642,0x062F,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord92[] = {0x0643,0x0627,0x0646,0x062A,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord93[] = {0x0644,0x0630,0x0644,0x0643,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord94[] = {0x0623,0x0645,0x0627,0x0645,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord95[] = {0x0647,0x0646,0x0627,0x0643,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord96[] = {0x0642,0x0628,0x0644,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord97[] = {0x0645,0x0639,0x0647,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord98[] = {0x064A,0x0648,0x0645,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord99[] = {0x0645,0x0646,0x0647,0x0627,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord100[] = {0x0625,0x0644,0x0649,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord101[] = {0x0625,0x0630,0x0627,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord102[] = {0x0647,0x0644,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord103[] = {0x062D,0x064A,0x062B,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord104[] = {0x0647,0x064A,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord105[] = {0x0627,0x0630,0x0627,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord106[] = {0x0627,0x0648,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord107[] = {0x0648,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord108[] = {0x0645,0x0627,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord109[] = {0x0644,0x0627,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord110[] = {0x0627,0x0644,0x064A,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord111[] = {0x0625,0x0644,0x064A,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord112[] = {0x0645,0x0627,0x0632,0x0627,0x0644,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord113[] = {0x0644,0x0627,0x0632,0x0627,0x0644,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord114[] = {0x0644,0x0627,0x064A,0x0632,0x0627,0x0644,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord115[] = {0x0645,0x0627,0x064A,0x0632,0x0627,0x0644,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord116[] = {0x0627,0x0635,0x0628,0x062D,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord117[] = {0x0623,0x0635,0x0628,0x062D,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord118[] = {0x0623,0x0645,0x0633,0x0649,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord119[] = {0x0627,0x0645,0x0633,0x0649,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord120[] = {0x0623,0x0636,0x062D,0x0649,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord121[] = {0x0627,0x0636,0x062D,0x0649,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord122[] = {0x0638,0x0644,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord123[] = {0x0645,0x0627,0x0628,0x0631,0x062D,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord124[] = {0x0645,0x0627,0x0641,0x062A,0x0626,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord125[] = {0x0645,0x0627,0x0627,0x0646,0x0641,0x0643,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord126[] = {0x0628,0x0627,0x062A,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord127[] = {0x0635,0x0627,0x0631,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord128[] = {0x0644,0x064A,0x0633,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord129[] = {0x0625,0x0646,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord130[] = {0x0643,0x0623,0x0646,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord131[] = {0x0644,0x064A,0x062A,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord132[] = {0x0644,0x0639,0x0644,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord133[] = {0x0644,0x0627,0x0633,0x064A,0x0645,0x0627,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord134[] = {0x0648,0x0644,0x0627,0x064A,0x0632,0x0627,0x0644,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord135[] = {0x0627,0x0644,0x062D,0x0627,0x0644,0x064A,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord136[] = {0x0636,0x0645,0x0646,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord137[] = {0x0627,0x0648,0x0644,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord138[] = {0x0648,0x0644,0x0647,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord139[] = {0x0630,0x0627,0x062A,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord140[] = {0x0627,0x064A,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord141[] = {0x0628,0x062F,0x0644,0x0627,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord142[] = {0x0627,0x0644,0x064A,0x0647,0x0627,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord143[] = {0x0627,0x0646,0x0647,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord144[] = {0x0627,0x0644,0x0630,0x064A,0x0646,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord145[] = {0x0641,0x0627,0x0646,0x0647,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord146[] = {0x0648,0x0627,0x0646,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord147[] = {0x0648,0x0627,0x0644,0x0630,0x064A,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord148[] = {0x0648,0x0647,0x0630,0x0627,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord149[] = {0x0644,0x0647,0x0630,0x0627,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord150[] = {0x0627,0x0644,0x0627,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord151[] = {0x0641,0x0643,0x0627,0x0646,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord152[] = {0x0633,0x062A,0x0643,0x0648,0x0646,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord153[] = {0x0645,0x0645,0x0627,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord154[] = {0x0623,0x0628,0x0648,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord155[] = {0x0628,0x0625,0x0646,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord156[] = {0x0627,0x0644,0x0630,0x064A,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord157[] = {0x0627,0x0644,0x064A,0x0647,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord158[] = {0x064A,0x0645,0x0643,0x0646,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord159[] = {0x0628,0x0647,0x0630,0x0627,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord160[] = {0x0644,0x062F,0x064A,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord161[] = {0x0648,0x0623,0x0646,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord162[] = {0x0648,0x0647,0x064A,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord163[] = {0x0648,0x0623,0x0628,0x0648,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord164[] = {0x0622,0x0644,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord165[] = {0x0627,0x0644,0x0630,0x064A,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord166[] = {0x0647,0x0646,NULL };
  const UINT64 Arabic_Stemmer_utf8::stopWord167[] = {0x0627,0x0644,0x0630,0x0649,NULL };
  const UINT64 *Arabic_Stemmer_utf8::allStopWords[] = {stopWord0,stopWord1,stopWord2,stopWord3,stopWord4,stopWord5,stopWord6,stopWord7,stopWord8,stopWord9,stopWord10,stopWord11,stopWord12,stopWord13,stopWord14,stopWord15,stopWord16,stopWord17,stopWord18,stopWord19,stopWord20,stopWord21,stopWord22,stopWord23,stopWord24,stopWord25,stopWord26,stopWord27,stopWord28,stopWord29,stopWord30,stopWord31,stopWord32,stopWord33,stopWord34,stopWord35,stopWord36,stopWord37,stopWord38,stopWord39,stopWord40,stopWord41,stopWord42,stopWord43,stopWord44,stopWord45,stopWord46,stopWord47,stopWord48,stopWord49,stopWord50,stopWord51,stopWord52,stopWord53,stopWord54,stopWord55,stopWord56,stopWord57,stopWord58,stopWord59,stopWord60,stopWord61,stopWord62,stopWord63,stopWord64,stopWord65,stopWord66,stopWord67,stopWord68,stopWord69,stopWord70,stopWord71,stopWord72,stopWord73,stopWord74,stopWord75,stopWord76,stopWord77,stopWord78,stopWord79,stopWord80,stopWord81,stopWord82,stopWord83,stopWord84,stopWord85,stopWord86,stopWord87,stopWord88,stopWord89,stopWord90,stopWord91,stopWord92,stopWord93,stopWord94,stopWord95,stopWord96,stopWord97,stopWord98,stopWord99,stopWord100,stopWord101,stopWord102,stopWord103,stopWord104,stopWord105,stopWord106,stopWord107,stopWord108,stopWord109,stopWord110,stopWord111,stopWord112,stopWord113,stopWord114,stopWord115,stopWord116,stopWord117,stopWord118,stopWord119,stopWord120,stopWord121,stopWord122,stopWord123,stopWord124,stopWord125,stopWord126,stopWord127,stopWord128,stopWord129,stopWord130,stopWord131,stopWord132,stopWord133,stopWord134,stopWord135,stopWord136,stopWord137,stopWord138,stopWord139,stopWord140,stopWord141,stopWord142,stopWord143,stopWord144,stopWord145,stopWord146,stopWord147,stopWord148,stopWord149,stopWord150,stopWord151,stopWord152,stopWord153,stopWord154,stopWord155,stopWord156,stopWord157,stopWord158,stopWord159,stopWord160,stopWord161,stopWord162,stopWord163,stopWord164,stopWord165,stopWord166,stopWord167,NULL};


//  // data tables
//    const char *Arabic_Stemmer_utf8_new::defarticles[] = {"ال", "وال","بال", "كال",
//                                           "فال", "لل",
//                                                          "\0"};

//    const char *Arabic_Stemmer_utf8_new::suffixes[] = {"ها","ان","ات","ون","ين","يه","ية",
//                                        "ه","ة","ي",
//                                                       "\0"};

//    const char *Arabic_Stemmer_utf8_new::stopwords[] =
//      { "ان","بعد", "ضد", "يلي", "الى", "في", "من", "حتى", "وهو", "يكون",
//        "به", "وليس", "أحد", "على", "وكان", "تلك", "كذلك", "التي", "وبين",
//        "فيها", "عليها", "إن", "وعلى", "لكن", "عن", "مساء", "ليس", "منذ",
//        "الذي", "أما", "حين", "ومن", "لا", "ليسب", "وكانت", "أي", "ما", "عنه",
//        "حول", "دون", "مع", "لكنه", "ولكن", "له", "هذا", "والتي","فقط", "ثم",
//        "هذه", "أنه", "تكون", "قد", "بين", "جدا", "لن", "نحو", "كان", "لهم",
//        "لأن", "اليوم", "لم", "هؤلاء", "فإن", "فيه", "ذلك", "لو", "عند",
//        "اللذين", "كل", "بد", "لدى", "وثي", "أن", "ومع", "فقد", "بل", "هو",
//        "عنها", "منه", "بها", "وفي", "فهو", "تحت", "لها", "أو", "إذ", "علي",
//        "عليه", "كما", "كيف", "هنا", "وقد", "كانت", "لذلك", "أمام", "هناك",
//        "قبل", "معه", "يوم", "منها", "إلى", "إذا", "هل", "حيث", "هي", "اذا",
//        "او", "و", "ما", "لا", "الي", "إلي", "مازال", "لازال", "لايزال",
//        "مايزال", "اصبح", "أصبح", "أمسى", "امسى", "أضحى", "اضحى", "ظل",
//        "مابرح", "مافتئ", "ماانفك", "بات", "صار", "ليس", "إن", "كأن",
//        "ليت", "لعل", "لاسيما", "ولايزال", "الحالي", "ضمن", "اول", "وله",
//        "ذات", "اي", "بدلا", "اليها", "انه", "الذين", "فانه", "وان",
//        "والذي", "وهذا", "لهذا", "الا", "فكان", "ستكون", "مما", "أبو",
//        "بإن", "الذي", "اليه", "يمكن", "بهذا", "لدي", "وأن", "وهي", "وأبو",
//        "آل", "الذي", "هن", "الذى", NULL };

    UINT64 Arabic_Stemmer_utf8::allArabicChars[] = {
        0x067E,
        0x0679,
        0x0686,
        0x0698,
        0x0688,
        0x06AF,
        0x06A9,
        0x0691,
        0x06BA,
        0x06BE,
        0x06C1,
        0x0621,
        0x0622,
        0x0623,
        0x0624,
        0x0625,
        0x0626,
        0x0627,
        0x0628,
        0x0629,
        0x062A,
        0x062B,
        0x062C,
        0x062D,
        0x062E,
        0x062F,
        0x0630,
        0x0631,
        0x0632,
        0x0633,
        0x0634,
        0x0635,
        0x0636,
        0x0637,
        0x0638,
        0x0639,
        0x063A,
        0x0641,
        0x0642,
        0x0643,
        0x0644,
        0x0645,
        0x0646,
        0x0647,
        0x0648,
        0x0649,
        0x064A,
        NULL
    };

//    const char *Arabic_Stemmer_utf8_new::allArabicChars[] = {"پ",
//    		"ٹ",
//    		"چ",
//    		"ژ",
//    		"ڈ",
//    		"گ",
//    		"ک",
//    		"ڑ",
//    		"ں",
//    		"ھ",
//    		"ہ",
//    		"ء",
//    		"آ",
//    		"أ",
//    		"ؤ",
//    		"إ",
//    		"ئ",
//    		"ا",
//    		"ب",
//    		"ة",
//    		"ت",
//    		"ث",
//    		"ج",
//    		"ح",
//    		"خ",
//    		"د",
//    		"ذ",
//    		"ر",
//    		"ز",
//    		"س",
//    		"ش",
//    		"ص",
//    		"ض",
//    		"ط",
//    		"ظ",
//    		"ع",
//    		"غ",
//    		"ف",
//    		"ق",
//    		"ك",
//    		"ل",
//    		"م",
//    		"ن",
//    		"ه",
//    		"و",
//    		"ى",
//    		"ي",
//    		NULL
//    };

    UINT64 Arabic_Stemmer_utf8::normChars[] = {
        0x067E,
        0x0679,
        0x0686,
        0x0698,
        0x0688,
        0x06AF,
        0x06A9,
        0x0691,
        0x06BA,
        0x06BE,
        0x06C1,
        0x0621,
        0x0627,
        0x0627,
        0x0624,
        0x0627,
        0x0626,
        0x0627,
        0x0628,
        0x0647,
        0x062A,
        0x062B,
        0x062C,
        0x062D,
        0x062E,
        0x062F,
        0x0630,
        0x0631,
        0x0632,
        0x0633,
        0x0634,
        0x0635,
        0x0636,
        0x0637,
        0x0638,
        0x0639,
        0x063A,
        0x0641,
        0x0642,
        0x0643,
        0x0644,
        0x0645,
        0x0646,
        0x0647,
        0x0648,
        0x064A,
        0x064A,
        NULL
    };

//    const char *Arabic_Stemmer_utf8_new::normChars[] = {
//    		"پ",
//    		"ٹ",
//    		"چ",
//    		"ژ",
//    		"ڈ",
//    		"گ",
//    		"ک",
//    		"ڑ",
//    		"ں",
//    		"ھ",
//    		"ہ",
//    		"ء",
//    		"ا",
//    		"ا",
//    		"ؤ",
//    		"ا",
//    		"ئ",
//    		"ا",
//    		"ب",
//    		"ه",
//    		"ت",
//    		"ث",
//    		"ج",
//    		"ح",
//    		"خ",
//    		"د",
//    		"ذ",
//    		"ر",
//    		"ز",
//    		"س",
//    		"ش",
//    		"ص",
//    		"ض",
//    		"ط",
//    		"ظ",
//    		"ع",
//    		"غ",
//    		"ف",
//    		"ق",
//    		"ك",
//    		"ل",
//    		"م",
//    		"ن",
//    		"ه",
//    		"و",
//    		"ي",
//    		"ي",
//    		NULL
//    };

    UINT64 Arabic_Stemmer_utf8::norm3Chars[] = {
        0x067E,
        0x0679,
        0x0686,
        0x0698,
        0x0688,
        0x06AF,
        0x06A9,
        0x0691,
        0x06BA,
        0x06BE,
        0x06C1,
        0x0627,
        0x0627,
        0x0627,
        0x0627,
        0x0627,
        0x0627,
        0x0627,
        0x0628,
        0x0647,
        0x062A,
        0x062B,
        0x062C,
        0x062D,
        0x062E,
        0x062F,
        0x0630,
        0x0631,
        0x0632,
        0x0633,
        0x0634,
        0x0635,
        0x0636,
        0x0637,
        0x0638,
        0x0639,
        0x063A,
        0x0641,
        0x0642,
        0x0643,
        0x0644,
        0x0645,
        0x0646,
        0x0647,
        0x0648,
        0x064A,
        0x064A,
        NULL
    };

//    const char *Arabic_Stemmer_utf8_new::norm3Chars[] = {
//    		"پ",
//    		"ٹ",
//    		"چ",
//    		"ژ",
//    		"ڈ",
//    		"گ",
//    		"ک",
//    		"ڑ",
//    		"ں",
//    		"ھ",
//    		"ہ",
//    		"ا",
//    		"ا",
//    		"ا",
//    		"ا",
//    		"ا",
//    		"ا",
//    		"ا",
//    		"ب",
//    		"ه",
//    		"ت",
//    		"ث",
//    		"ج",
//    		"ح",
//    		"خ",
//    		"د",
//    		"ذ",
//    		"ر",
//    		"ز",
//    		"س",
//    		"ش",
//    		"ص",
//    		"ض",
//    		"ط",
//    		"ظ",
//    		"ع",
//    		"غ",
//    		"ف",
//    		"ق",
//    		"ك",
//    		"ل",
//    		"م",
//    		"ن",
//    		"ه",
//    		"و",
//    		"ي",
//    		"ي",
//    		NULL
//    };

    // SDH  2/19/2012
    // Note that the nulls in this table should be messed with carefully!!! BUT they are not currently in use.
    // Since they are of the same size as allArabicChars, we can rely on the null in that table.
    // However the best way to do this correctly would be to regenerate an allVowels table from the allArabicChars
    // table, with only non-zero elements in this table.
    UINT64 Arabic_Stemmer_utf8::arabicVowelChars[] = {
        CHAR_NULL,
        CHAR_NULL,
        CHAR_NULL,
        CHAR_NULL,
        CHAR_NULL,
        CHAR_NULL,
        CHAR_NULL,
        CHAR_NULL,
        CHAR_NULL,
        CHAR_NULL,
        CHAR_NULL,
        0x0621,
        0x0622,
        0x0623,
        0x0624,
        0x0625,
        0x0626,
        0x0627,
        CHAR_NULL,
        CHAR_NULL,
        CHAR_NULL,
        CHAR_NULL,
        CHAR_NULL,
        CHAR_NULL,
        CHAR_NULL,
        CHAR_NULL,
        CHAR_NULL,
        CHAR_NULL,
        CHAR_NULL,
        CHAR_NULL,
        CHAR_NULL,
        CHAR_NULL,
        CHAR_NULL,
        CHAR_NULL,
        CHAR_NULL,
        CHAR_NULL,
        CHAR_NULL,
        CHAR_NULL,
        CHAR_NULL,
        CHAR_NULL,
        CHAR_NULL,
        CHAR_NULL,
        CHAR_NULL,
        CHAR_NULL,
        0x0648,
        0x0649,
        0x064A,
        NULL
    };

//    const char *Arabic_Stemmer_utf8_new::arabicVowelChars[] = {
//    		" ",
//    		" ",
//    		" ",
//    		" ",
//    		" ",
//    		" ",
//    		" ",
//    		" ",
//    		" ",
//    		" ",
//    		" ",
//    		"ء",
//    		"آ",
//    		"أ",
//    		"ؤ",
//    		"إ",
//    		"ئ",
//    		"ا",
//    		" ",
//    		" ",
//    		" ",
//    		" ",
//    		" ",
//    		" ",
//    		" ",
//    		" ",
//    		" ",
//    		" ",
//    		" ",
//    		" ",
//    		" ",
//    		" ",
//    		" ",
//    		" ",
//    		" ",
//    		" ",
//    		" ",
//    		" ",
//    		" ",
//    		" ",
//    		" ",
//    		" ",
//    		" ",
//    		" ",
//    		"و",
//    		"ى",
//    		"ي",
//    		NULL
//    };

    Arabic_Stemmer_utf8::stem_info_u_t Arabic_Stemmer_utf8::stemtable_unicode[NUMSTEMMERS] = {
      {"none", &Arabic_Stemmer_utf8::no_stem_unicode},
      {"arabic_stop", &Arabic_Stemmer_utf8::arabic_stop_unicode},
      {"arabic_norm2", &Arabic_Stemmer_utf8::arabic_norm2_unicode},
      {"arabic_norm2_stop", &Arabic_Stemmer_utf8::arabic_norm2_stop_unicode},
      {"arabic_light10", &Arabic_Stemmer_utf8::arabic_light10_unicode},
      {"arabic_light10_stop", &Arabic_Stemmer_utf8::arabic_light10_stop_unicode}
    };

    Arabic_Stemmer_utf8::Arabic_Stemmer_utf8(std::string stemFunc) {
      // initialize stemmer function and stopwords list.
      stem_fct_unicode = NULL;
      for (int i = 0; i < NUMSTEMMERS; i++) {
        if (stemFunc == stemtable_unicode[i].option)
          // call with (this->*stem_fct)(args)
          stem_fct_unicode = stemtable_unicode[i].stem_fct_unicode;
      }
      if (stem_fct_unicode == NULL) {
        // bad things here, throw an Exception.
        LEMUR_THROW(LEMUR_BAD_PARAMETER_ERROR, "Arabic_Stemmer_utf8: unknown stem function: "+stemFunc);
      }
      for (int i = 0; allStopWords[i]; i++) {
        stop_words_set_unicode.insert(allStopWords[i]);
      }

      createStemmerTransitionTables();
    }

    Arabic_Stemmer_utf8::~Arabic_Stemmer_utf8() {
    }

    void Arabic_Stemmer_utf8::createStemmerTransitionTables(){
    	// The arrays are "null-terminated" and all of the same size
    	for (int i = 0; allArabicChars[i]; i++) {
            UINT64 originalChar = allArabicChars[i];            
            normCharTable[originalChar] = normChars[i];
            norm3CharTable[originalChar] = norm3Chars[i];

            // Only add to the vowels table if it's non-null
            if (arabicVowelChars[i]) {
                arabicVowelTable[originalChar] = arabicVowelChars[i];
            }
    	}
    }

//////////////////////////////  UNICODE VERSIONS of stemming functions ///////////////////////////////////


    /**************************************************************************/
    /********************** UNICODE VERSIONS of *******************************/
    /**********************  STEMMING FUNCTIONS *******************************/
    /**************************************************************************/

    /************************************************************
     *  REMOVE_DEFINITE_ARTICLES
     *  corrected version of REMOVE_DEFINITE_ARTICLE
     *     should be done after alef's are normalized
     ************************************************************/
    void Arabic_Stemmer_utf8::remove_definite_articles_unicode(UINT64 *word, UINT64 *result) {
      size_t len;
      size_t wordlen = Uint64Comp::u_strlen(word);
      const UINT64 **nextart = allDefArticles;
      Uint64Comp::u_strcpy(result, word);

      for ( ; *nextart != NULL ; nextart++) {
          len = Uint64Comp::u_strlen(*nextart);
          if (wordlen > len+1 && Uint64Comp::u_strncmp(word, *nextart, len)==0) {
            Uint64Comp::u_strcpy(result, word+len);
            break ;  // only want to find one
          }
      }
    }

    /************************************************************
     *  REMOVE_suffixes
     *     should be done after alef's are normalized and prefixes removed
     *    this removes one suffix
     *    REMOVE_ALL_SUFFIXES removes all suffixes, order is significant
     ************************************************************/
    void Arabic_Stemmer_utf8::remove_all_suffixes_unicode(UINT64 *word, UINT64 *result,
                             size_t lenlimit) /* min size for remainder */ {
      size_t suflen;
      size_t wordlen = Uint64Comp::u_strlen(word);
      const UINT64 **nextsuf = allSuffixes;

      if (wordlen == 0) {
        *result = CHAR_NULL;
        return;
      }
      Uint64Comp::u_strcpy (result, word);

      for ( ; *nextsuf != NULL ; nextsuf++) {
        suflen = Uint64Comp::u_strlen(*nextsuf);
        wordlen = Uint64Comp::u_strlen(result);
        if (wordlen > suflen+lenlimit &&
            Uint64Comp::u_strncmp(word+wordlen-suflen, *nextsuf, suflen) == 0) {
          result[wordlen-suflen] = CHAR_NULL;
        }
      }
    }

    ////////// **************** ////////////

    void Arabic_Stemmer_utf8::no_stem_unicode(UINT64 *word, UINT64 *result) {
      Uint64Comp::u_strcpy(result, word);
    }

    /* .  If the current word is a stopword then return true. */
    bool Arabic_Stemmer_utf8::on_stop_list_unicode (UINT64 *word) {
      return (stop_words_set_unicode.find(word) != stop_words_set_unicode.end());
    }

    void Arabic_Stemmer_utf8::arabic_stop_unicode (UINT64 *word, UINT64 *result) {
      if (on_stop_list_unicode(word))
          *result = CHAR_NULL;
      else Uint64Comp::u_strcpy(result, word);
    }

    /**********************************************************/
    // Version of normalization that uses NormChar table - now with unicode

    // Skips any character which is not ASCII or arabic
    void Arabic_Stemmer_utf8::arabic_norm2_unicode(UINT64 *word, UINT64 *result) {
      UINT64 *i = word ;
      UINT64 *o = result ;

      transitionTable::iterator it;

      while (*i) {
        
        if (*i < ARABIC_RANGE_LOW || *i > ARABIC_RANGE_HIGH)
          *o++ = *i;
        else if ((it = normCharTable.find(*i)) != normCharTable.end()) 
          {
            *o++ = it->second;
          }

        i++;
      }
      *o = CHAR_NULL;
    }

    void Arabic_Stemmer_utf8::arabic_norm2_stop_unicode (UINT64 *word, UINT64 *result) {
      arabic_norm2_unicode(word, result);
      if (on_stop_list_unicode(result))
        *result = CHAR_NULL;
    }

    void Arabic_Stemmer_utf8::arabic_light10_unicode (UINT64 *word, UINT64 *result) {
      UINT64 *tempstring=Uint64Comp::u_strdup(word) ;
      UINT64 *tempstring2=Uint64Comp::u_strdup(word);

      UINT64 *begin ;
      *tempstring2 = CHAR_NULL;
      arabic_norm2_unicode (word, tempstring) ;

      // Should remove stop words here

      begin = tempstring;

      // Remove waw (advance pointer) if remainder >= 3 chars
      if (*tempstring == CHAR_WAW && Uint64Comp::u_strlen(tempstring) > 3) begin++ ;

      // Remove definite article if remainder >=2 chars
      remove_definite_articles_unicode(begin, tempstring2);

      // Remove suffixes from small suffix list
      if (Uint64Comp::u_strlen(tempstring2) > 2)
        remove_all_suffixes_unicode(tempstring2, result, (size_t)1);
      else
        Uint64Comp::u_strcpy(result, tempstring2);
      free (tempstring) ;
      free (tempstring2) ;
    }

    void Arabic_Stemmer_utf8::arabic_light10_stop_unicode (UINT64 *word, UINT64 *result) {
      UINT64 *tempstring = Uint64Comp::u_strdup(word);
      UINT64 *tempstring2 = Uint64Comp::u_strdup(word);

      UINT64 *begin ;
      *tempstring2 = CHAR_NULL;
      arabic_norm2_unicode (word, tempstring) ;

      if (on_stop_list_unicode(tempstring)) {
        *result = CHAR_NULL;
        free (tempstring) ;
        free (tempstring2) ;
        return;
      }
      // Not a stop word
      begin = tempstring;
      // Remove waw (advance pointer) if remainder >= 3 chars
      if (*tempstring == CHAR_WAW && Uint64Comp::u_strlen(tempstring) > 3) begin++ ;

      // Remove definite article if remainder >=2 chars
      remove_definite_articles_unicode(begin, tempstring2);

      // Remove suffixes from small suffix list
      if (Uint64Comp::u_strlen(tempstring2) > 2)
        remove_all_suffixes_unicode(tempstring2, result, (size_t)1);
      else
        Uint64Comp::u_strcpy(result, tempstring2);
      free (tempstring) ;
      free (tempstring2) ;
    }

/////////////     INTERFACE USES char * which is UTF-8  ////////////////

    void Arabic_Stemmer_utf8::stemTerm(char *word, char *result) {
      
        // Only place that the "real" strlen should be used!
        int len = strlen( word );

        UINT64* u_word = new UINT64[len + 1];
        int* offsets = new int[len + 1];
        int* lengths = new int[len + 1];

        // Decode UTF-8 into Unicode
        _transcoder.utf8_decode( word, &u_word, NULL, NULL,
                                 &offsets, &lengths );

        UINT64* u_result = new UINT64[len + 1];

        // Unicode-friendly stemming
        (this->*stem_fct_unicode)(u_word, u_result);

        // Need to convert back from Unicode to UTF8 - using the encoding function
        // NOTE: Decode does an entire string, whereas Encode does a single unicode code point only
        int buf_index = 0;
        int byte_offset = 0;
        int j;
        for (j = 0; j < len && u_result[j]; j++ ) {
          
            // Re-encode and copy into new buffer
            _transcoder.utf8_encode( u_result[j],
                                     result + buf_index, &byte_offset );
            buf_index += byte_offset;
        }
        result[buf_index] = '\0';
        
        delete[] u_word;
        delete[] offsets;
        delete[] lengths;
        delete[] u_result;
    }
  }
}
