/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#pragma once

#include <string>
#include <eosio/varint.hpp>
#include <eosio/privileged.hpp>

namespace eosio {

   using std::string;


   template<typename T>
   void push(T&){}

   template<typename Stream, typename T, typename ... Types>
   void push(Stream &s, T arg, Types ... args){
      s << arg;
      push(s, args...);
   }

   template<class ... Types> checksum256 get_checksum256(const Types & ... args ){
      datastream <size_t> ps;
      push(ps, args...);
      size_t size = ps.tellp();

      std::vector<char> result;
      result.resize(size);

      datastream<char *> ds(result.data(), result.size());
      push(ds, args...);
      checksum256 digest=sha256(result.data(), result.size());
      return digest;
   }

   // inline bool is_equal_capi_checksum256( checksum256 a, checksum256 b ){
   //    return std::memcmp( a.hash, b.hash, 32 ) == 0;
   // }
}
