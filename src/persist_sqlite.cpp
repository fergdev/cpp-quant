#include "persist_sqlite.hpp"
#include <iostream>
#include <thread>

static bool open_db(sqlite3** pdb){
  if (sqlite3_open("cpp_quant.db", pdb) != SQLITE_OK) return false;
  char* err=nullptr;
  sqlite3_exec(*pdb,"PRAGMA journal_mode=WAL",nullptr,nullptr,&err);
  sqlite3_exec(*pdb,"CREATE TABLE IF NOT EXISTS fills(id TEXT, qty REAL, px REAL, ts INTEGER)",nullptr,nullptr,&err);
  sqlite3_exec(*pdb,"CREATE TABLE IF NOT EXISTS ticks(sym TEXT, px REAL, ts INTEGER)",nullptr,nullptr,&err);
  return true;
}

Persist::Persist(Channel<Tick,8192>& t, Channel<OrderResp,4096>& f)
  : ticks(t), fills(f) {}

void Persist::run() {
  if (!open_db(&db)) { std::cerr << "sqlite open failed\n"; return; }
  Tick t; OrderResp e;
  for (;;) {
    bool did=false;
    if (ticks.pop(t)) {
      did=true;
      sqlite3_stmt* st; sqlite3_prepare_v2(db,"INSERT INTO ticks VALUES(?,?,?)",-1,&st,nullptr);
      sqlite3_bind_text(st,1,t.sym.c_str(),-1,SQLITE_TRANSIENT);
      sqlite3_bind_double(st,2,t.last);
      sqlite3_bind_int64(st,3,t.ts_ns);
      sqlite3_step(st); sqlite3_finalize(st);
    }
    if (fills.pop(e)) {
      did=true;
      sqlite3_stmt* st; sqlite3_prepare_v2(db,"INSERT INTO fills VALUES(?,?,?,?)",-1,&st,nullptr);
      sqlite3_bind_text(st,1,e.id.c_str(),-1,SQLITE_TRANSIENT);
      sqlite3_bind_double(st,2,e.filled_qty);
      sqlite3_bind_double(st,3,e.avg_px);
      sqlite3_bind_int64(st,4,e.ts_ns);
      sqlite3_step(st); sqlite3_finalize(st);
    }
    if (!did) std::this_thread::yield();
  }
}
