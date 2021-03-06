 // Copyrightgg 1996-2012, 2014, 2016-2018 by Jon Dart.  All Rights Reserved.

#include "globals.h"
#include "hash.h"
#include "bitprobe.h"
#include "scoring.h"
#include "bitbase.cpp"
#ifdef SYZYGY_TBS
#include "syzygy.h"
#endif

#ifdef SYZYGY_TBS
static bool tb_init = false;

bool tb_init_done()
{
   return tb_init;
}
int EGTBMenCount = 0;
#endif

MoveArray *gameMoves;
Options options;
BookReader openingBook;
Log *theLog = nullptr;
string learnFileName;
LockDefine(input_lock);
#ifdef TUNE
Tune tune_params;
#endif

static const char * LEARN_FILE_NAME = "arasan.lrn";

static const char * DEFAULT_BOOK_NAME = "book.bin";

static const char * RC_FILE_NAME = "arasan.rc";

#ifdef UCI_LOG
fstream ucilog;
#endif

string programPath;

string derivePath(const char *fileName) {
   return derivePath(programPath.c_str(),fileName);
}

string derivePath(const char *base, const char *fileName) {
    string result(base);
    size_t pos;
#ifdef _WIN32
    pos = result.rfind('\\',string::npos);
#else
    pos = result.rfind('/',string::npos);
#endif
    if (pos == string::npos) {
      return fileName;
    }
    else {
      return result.substr(0,pos+1) + fileName;
    }
}

int initGlobals(const char *pathName, bool initLog) {
   programPath = pathName;
   gameMoves = new MoveArray();
   if (initLog) {
       theLog = new Log();
       theLog->clear();
       theLog->write_header();
   }
   LockInit(input_lock);
#ifdef UCI_LOG
   ucilog.open(derivePath("ucilog").c_str(),ios::out|ios::app);
   ucilog << "starting up" << endl;
#endif
#if defined(_MSC_VER) && (_MSC_VER <1800)
   tb_init_map[Options::TbType::None] =
       tb_init_map[Options::TbType::NalimovTb] =
       tb_init_map[Options::TbType::GaviotaTb] =
       tb_init_map[Options::TbType::SyzygyTb] = false;
#endif
   return 1;
}

void CDECL cleanupGlobals(void) {
   openingBook.close();
   delete gameMoves;
   delete theLog;
   LockFree(input_lock);
   Scoring::cleanup();
#ifdef UCI_LOG
    ucilog.close();
#endif
#ifdef GAVIOTA_TBS
   GaviotaTb::freeTB();
#endif
}

void initOptions(const char *pathName) {
    programPath = pathName;
    string rcPath = derivePath(RC_FILE_NAME);
    // try to read arasan.rc file
    options.init(rcPath);
#ifndef _WIN32
    // Also read .rc from the user's HOME,
    // if there is one.
    if (getenv("HOME")) {
       rcPath = derivePath(getenv("HOME"),RC_FILE_NAME);
       options.init(rcPath);
    }
#endif
    learnFileName = derivePath(LEARN_FILE_NAME);
}

void delayedInit() {
#ifdef SYZYGY_TBS
   if (options.search.use_tablebases && !tb_init_done()) {
       EGTBMenCount = 0;
       string path;
       if (options.search.syzygy_path == "") {
          options.search.syzygy_path=derivePath("syzygy");
       }
       path = options.search.syzygy_path;
       EGTBMenCount = SyzygyTb::initTB(options.search.syzygy_path);
       tb_init = true;
       if (EGTBMenCount) {
          stringstream msg;
          msg << "found " << EGTBMenCount << "-man Syzygy tablebases in directory " << path << endl;
          cerr << msg.str();
#ifdef UCI_LOG
          ucilog << msg.str();
#endif
       }
    }
#endif
    // also initialize the book here
    if (options.book.book_enabled && !openingBook.is_open()) {
        openingBook.open(derivePath(DEFAULT_BOOK_NAME).c_str());
    }
}

   void unloadTb() {
#ifdef SYZYGY_TBS
   if (tb_init_done()) {
      // Note: Syzygy code will free any existing memory if
      // initialized more than once. So no need to do anything here.
      tb_init = false;
   }
#endif
}

