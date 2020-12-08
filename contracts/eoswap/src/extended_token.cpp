#include <common/extended_token.hpp>

void extended_token::create(const name& issuer, const extended_asset& maximum_supply) {
   if (auth_mode) {
      require_auth(get_self());
   }
   auto sym = maximum_supply.quantity.symbol;
   check(sym.is_valid(), "invalid symbol name");
   check(maximum_supply.quantity.is_valid(), "invalid supply");
   check(maximum_supply.quantity.amount > 0, "max-supply must be positive");

   stats statstable(get_self(), maximum_supply.contract.value);
   auto  existing = statstable.find(sym.code().raw());
   check(existing == statstable.end(), "token with symbol already exists");

   statstable.emplace(get_self(), [&](auto& s) {
      s.supply.symbol = maximum_supply.quantity.symbol;
      s.max_supply    = maximum_supply.quantity;
      s.issuer        = issuer;
   });
}

void extended_token::issue(const name& to, const extended_asset& quantity, const string& memo) {
   auto sym = quantity.quantity.symbol;
   check(sym.is_valid(), "invalid symbol name");
   check(memo.size() <= 256, "memo has more than 256 bytes");

   stats statstable(get_self(), quantity.contract.value);
   auto  existing = statstable.find(sym.code().raw());
   check(existing != statstable.end(), "extended_token with symbol does not exist, create extended_token before issue");
   const auto& st = *existing;
   //    check(to == st.issuer, "tokens can only be issued to issuer account");
   if (auth_mode) {
      require_auth(st.issuer);
   }
   check(quantity.quantity.is_valid(), "invalid quantity");
   check(quantity.quantity.amount > 0, "must issue positive quantity");

   check(quantity.quantity.symbol == st.supply.symbol, "symbol precision mismatch");
   check(quantity.quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");

   statstable.modify(st, same_payer, [&](auto& s) { s.supply += quantity.quantity; });

   name ram_payer = get_self();
   if (auth_mode) {
      ram_payer = st.issuer;
   }

   add_balance(to, quantity, ram_payer);
}

void extended_token::retire(const extended_asset& quantity, const string& memo) {
   auto sym = quantity.quantity.symbol;
   check(sym.is_valid(), "invalid symbol name");
   check(memo.size() <= 256, "memo has more than 256 bytes");

   stats statstable(get_self(), quantity.contract.value);
   auto  existing = statstable.find(sym.code().raw());
   check(existing != statstable.end(), "token with symbol does not exist");
   const auto& st = *existing;
   if (auth_mode) {
      require_auth(st.issuer);
   }
   check(quantity.quantity.is_valid(), "invalid quantity");
   check(quantity.quantity.amount > 0, "must retire positive quantity");

   check(quantity.quantity.symbol == st.supply.symbol, "symbol precision mismatch");

   statstable.modify(st, same_payer, [&](auto& s) { s.supply -= quantity.quantity; });

   sub_balance(st.issuer, quantity);
}

void extended_token::transfer(const name& from, const name& to, const extended_asset& quantity, const string& memo) {
   check(from != to, "cannot transfer to self");
   if (auth_mode) {
      require_auth(from);
   }
   check(is_account(to), "to account does not exist");
   auto        sym = quantity.quantity.symbol.code();
   stats       statstable(get_self(), quantity.contract.value);
   const auto& st = statstable.get(sym.raw());

   require_recipient(from);
   require_recipient(to);

   check(quantity.quantity.is_valid(), "invalid quantity");
   check(quantity.quantity.amount > 0, "must transfer positive quantity");
   check(quantity.quantity.symbol == st.supply.symbol, "symbol precision mismatch");
   check(memo.size() <= 256, "memo has more than 256 bytes");

   auto payer = has_auth(to) ? to : from;

   sub_balance(from, quantity);
   add_balance(to, quantity, payer);
}

void extended_token::sub_balance(const name& owner, const extended_asset& value) {
   accounts from_acnts(get_self(), owner.value);
   auto     idx = from_acnts.get_index<"byextasset"_n>();
   auto     it  = idx.find(to_namesym(value.get_extended_symbol()));
   check(it != idx.end(), "extended_symbol does not exist");
   auto from = *it;
   //    const auto& from = from_acnts.get(value.symbol.code().raw(), "no balance object found");
   check(from.balance.quantity.amount >= value.quantity.amount, "overdrawn balance");

   auto its = from_acnts.find(it->sequence);
   check(its != from_acnts.end(), "extended_symbol does not exist");
   name ram_payer = get_self();
   if (auth_mode) {
      ram_payer = owner;
   }
   from_acnts.modify(its, ram_payer, [&](auto& a) { a.balance -= value; });
}

void extended_token::add_balance(const name& owner, const extended_asset& value, const name& ram_payer) {
   accounts to_acnts(get_self(), owner.value);
   auto     idx = to_acnts.get_index<"byextasset"_n>();
   auto     to  = idx.find(to_namesym(value.get_extended_symbol()));

   //    auto     to = to_acnts.find(value.symbol.code().raw());
   name _ram_payer = get_self();
   if (auth_mode) {
      _ram_payer = ram_payer;
   }
   if (to == idx.end()) {
      to_acnts.emplace(_ram_payer, [&](auto& a) { a.balance = value; });
   } else {
      auto its = to_acnts.find(to->sequence);
      check(its != to_acnts.end(), "extended_symbol does not exist");
      to_acnts.modify(its, same_payer, [&](auto& a) { a.balance += value; });
   }
}

void extended_token::open(const name& owner, const extended_symbol& symbol, const name& ram_payer) {
   if (auth_mode) {
      require_auth(ram_payer);
   }

   check(is_account(owner), "owner account does not exist");

   auto        sym_code_raw = symbol.get_symbol().code().raw();
   stats       statstable(get_self(), symbol.get_contract().value);
   const auto& st = statstable.get(sym_code_raw, "symbol does not exist");
   check(st.supply.symbol == symbol.get_symbol(), "symbol precision mismatch");

   accounts acnts(get_self(), owner.value);
   auto     idx = acnts.get_index<"byextasset"_n>();
   auto     it  = idx.find(to_namesym(symbol));
   if (it == idx.end()) {
      acnts.emplace(ram_payer, [&](auto& a) { a.balance = extended_asset{0, symbol}; });
   }
}

void extended_token::close(const name& owner, const extended_symbol& symbol) {
   if (auth_mode) {
      require_auth(owner);
   }
   accounts acnts(get_self(), owner.value);
   auto     idx = acnts.get_index<"byextasset"_n>();
   auto     it  = idx.find(to_namesym(symbol));
   check(it != idx.end(), "Balance row already deleted or never existed. Action won't have any effect.");
   check(it->balance.quantity.amount == 0, "Cannot close because the balance is not zero.");

   auto its = acnts.find(it->sequence);
   acnts.erase(its);
}
