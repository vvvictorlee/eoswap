#pragma once
#include <common/BType.hpp>

class transfer_mgmt {
private:
  name self;

public:
   static constexpr eosio::name token_account{"eosio.token"_n};
   static constexpr eosio::name active_permission{"active"_n};
  // constructor
  //-------------------------------------------------------------------------
  transfer_mgmt(name _self) : self(_self) {}

  template <typename T>
  void eosiotoken_transfer(name from, name to, asset quantity, std::string memo,
                           T func) {
    eosio::check(quantity.symbol == eosio::symbol("EOS", 4),
                 "only accepts EOS for deposits");
    eosio::check(quantity.is_valid(), "Invalid token transfer");
    eosio::check(quantity.amount > 0, "Quantity must be positive");

    if (from == self) {
      check_blacklist(to);
      return;
    }

    if (to != self) {
      check_blacklist(from);
      return;
    }

    // system account could transfer eos to contract
    // eg) unstake, sellram, etc
    if (is_system_account(from)) {
      return;
    }
    transfer_info res;
    size_t n1 = memo.find(':');
    size_t n2 = memo.find(':', n1 + 1);
    res.from = from;
    res.action = memo.substr(0, n1);

    if (n2 == std::string::npos) {
      res.param = memo.substr(n1 + 1);
    } else {
      // param:type:seller:block:checksum
      size_t n3 = memo.find(':', n2 + 1);
      size_t n4 = memo.find(':', n3 + 1);
      size_t n5 = memo.find(':', n4 + 1);
      res.param = memo.substr(n1 + 1, n2 - (n1 + 1));
      res.type = atoi(memo.substr(n2 + 1, n3 - (n2 + 1)).c_str());
      res.seller = name(atoll(memo.substr(n3 + 1, n4 - (n3 + 1)).c_str()));
      res.block = (uint32_t)atoll(memo.substr(n4 + 1, n5 - (n4 + 1)).c_str());
      res.checksum = (uint32_t)atoll(memo.substr(n5 + 1).c_str());
    }

    res.quantity = quantity;
    func(res);
  }

  template <typename T>
  void non_eosiotoken_transfer(name from, name to, asset quantity,
                               std::string memo, T func) {
    eosio::check(quantity.symbol == eosio::symbol("EOS", 4),
                 "only accepts EOS for deposits");
    eosio::check(quantity.is_valid(), "Invalid token transfer");
    eosio::check(quantity.amount > 0, "Quantity must be positive");

    if (from == self) {
      check_blacklist(to);
      return;
    }

    if (to != self) {
      check_blacklist(from);
      return;
    }

    transfer_info res;
    size_t n1 = memo.find(':');
    size_t n2 = memo.find(':', n1 + 1);
    res.from = from;
    res.action = memo.substr(0, n1);

    if (n2 == std::string::npos) {
      res.param = memo.substr(n1 + 1);
    } else {
      // param:type:seller:block:checksum
      size_t n3 = memo.find(':', n2 + 1);
      size_t n4 = memo.find(':', n3 + 1);
      size_t n5 = memo.find(':', n4 + 1);
      res.param = memo.substr(n1 + 1, n2 - (n1 + 1));
      res.type = atoi(memo.substr(n2 + 1, n3 - (n2 + 1)).c_str());
      res.seller = name(atoll(memo.substr(n3 + 1, n4 - (n3 + 1)).c_str()));
      res.block = (uint32_t)atoll(memo.substr(n4 + 1, n5 - (n4 + 1)).c_str());
      res.checksum = (uint32_t)atoll(memo.substr(n5 + 1).c_str());
    }

    res.quantity = quantity;
    func(res);
  }

  bool is_system_account(name name) {
    if (name == "eosio.bpay"_n || name == "eosio.msig"_n ||
        name == "eosio.names"_n || name == "eosio.ram"_n ||
        name == "eosio.ramfee"_n || name == "eosio.saving"_n ||
        name == "eosio.stake"_n || name == "eosio.token"_n ||
        name == "eosio.vpay"_n) {
      return true;
    }
    return false;
  }

  void check_blacklist(name from) {
    if (from == "blacklist"_n) {
      check(false, "blacklist rejected");
    }
  }

  symbol core_symbol() const {
    symbol _core_symbol = symbol(symbol_code("EOS"), 4);
    return _core_symbol;
  };
  void on_transfer(name from, name to, asset quantity, std::string memo) {

    //  check(get_first_receiver() == "eosio.token"_n, "should be eosio.token");
    print_f("On notify : % % % %", from, to, quantity, memo);
    if (from == self || to != self || quantity.symbol != core_symbol() ||
        memo.empty()) {
      // print("memo is empty on trasfer");
      return;
    }
    check(quantity.is_valid(), "invalid quantity");
    check(quantity.amount > 0, "must transfer positive quantity");

    //  check(quantity.amount>100, "amount could not be less than 100");
    //
    // check(find(to) != end, "no account's business found ");

    std::vector<std::string> parameters = get_parameters(memo);
    check(parameters.size() > 0, "parse memo failed ");
    uint64_t transfer_category = convert_to_int(parameters[index_category]);

    auto check_parameters_size = [&](uint64_t category) -> bool {
      std::vector<uint8_t> index_counts = {};
      std::vector<std::string> help_strings = {
          "stake_category,index_id",
          "pay_category,index_id",
          "deposit_category,from,to,notify ",
          "appeal_category,service_id ,evidence,info,reason,provider",
          "arbitrator_category,type ",
          "resp_case_category,arbitration_id,evidence",
          "risk_guarantee_category,id,duration"};

      // check(category >= 0 && category < index_counts.size(), "unknown
      // category"); check(parameters.size() == index_counts[category],
      //       "the parameters'size does not match ");

      if (category >= 0 && category < index_counts.size()) {
        if (parameters.size() == index_counts[category]) {
          return true;
        }
        std::string str =
            "the parameters'size does not match " + help_strings[category];
        check(false, str.c_str());
      }

      return false;
    };

    if (!check_parameters_size(transfer_category)) {
      print("unknown category");
      return;
    }

    auto s2name = [&](uint64_t index) -> name {
      if (index >= 0 && index < parameters.size()) {
        return name(parameters[index]);
      } else {
        print("index invalid", index, parameters.size());
      }

      return name{};
    };
    auto s2int = [&](uint64_t index) -> uint64_t {
      if (index >= 0 && index < parameters.size()) {
        return convert_to_int(parameters[index]);
      } else {
        print("index invalid", index, parameters.size());
      }
      return 0;
    };

    if (tc_deposit == transfer_category) {
      //   call_deposit(s2name(static_cast<uint64_t>(index_from)),
      //   s2name(index_to), quantity, 0 != s2int(index_notify));
      // transfer(_self, riskctrl_account, quantity, memo);
    } else {

      name account = from;
      //       switch (transfer_category) {
      //       case tc_service_stake:
      //          stake_asset(s2int(index_id), account, quantity, memo);
      //          // oracle_transfer(_self, provider_account, quantity, memo,
      //          true); break;
      //       case tc_pay_service:
      //          pay_service(s2int(index_id), account, quantity);
      //          // oracle_transfer(_self, consumer_account, quantity, memo,
      //          true); break;
      //       case tc_arbitration_stake_appeal:
      //          _appeal(account, s2int(index_id), quantity,
      //          parameters[index_reason], parameters[index_evidence],
      //          s2int(index_provider));
      //          // oracle_transfer(_self, arbitrat_account, quantity, memo,
      //          true); break;
      //       case tc_arbitration_stake_arbitrator:
      //          _regarbitrat(account, s2int(index_type), quantity, "");
      //          // oracle_transfer(_self, arbitrat_account, quantity, memo,
      //          true); break;
      //       case tc_arbitration_stake_resp_case:
      //          _respcase(account, s2int(index_id), quantity,
      //          parameters[index_evidence]);
      //          // oracle_transfer(_self, arbitrat_account, quantity, memo,
      //          true); break;
      //       case tc_risk_guarantee:
      //          add_guarantee(s2int(index_id), account, quantity,
      //          s2int(index_duration));
      //          // oracle_transfer(_self, arbitrat_account, quantity, memo,
      //          true); break;
      //       default:
      //          //  check(false, "unknown  transfer category ");
      //          print("unknown  transfer category ");
      //          break;
      //       }
    }
  }

  /**
   * @brief
   *
   * @param from
   * @param to
   * @param quantity
   * @param memo
   */
  void transfer(name from, name to, asset quantity, std::string memo) {
    oracle_transfer(from, to, quantity, memo, true);
  }

  void oracle_transfer(name from, name to, asset quantity, std::string memo,
                       bool is_deferred) {

    check(from != to, "cannot transfer to self");
    //  require_auth( from );
    check(is_account(to), "to account does not exist");
    //  auto sym = quantity.symbol.code();
    //  stats statstable( _self, sym.raw() );
    //  const auto& st = statstable.get( sym.raw() );

    //  require_recipient( from );
    //  require_recipient( to );

    check(quantity.is_valid(), "invalid quantity");
    check(quantity.amount > 0, "must transfer positive quantity");
    // check(quantity.symbol == st.supply.symbol, "symbol precision mismatch");
    check(memo.size() <= 256, "memo has more than 256 bytes");

    //   token::transfer_action transfer_act{ token_account, { account,
    //   active_permission } };
    //          transfer_act.send( account, consumer_account, amount, memo );

    //  auto payer = has_auth( to ) ? to : from;
    // print("===quantity");
    quantity.print();

    if (!is_deferred) {
      action(permission_level{from, "active"_n}, token_account, "transfer"_n,
             std::make_tuple(from, to, quantity, memo))
          .send();
    } else {
      transaction t;
      t.actions.emplace_back(permission_level{from, active_permission},
                             token_account, "transfer"_n,
                             std::make_tuple(from, to, quantity, memo));
      t.delay_sec = 0;
      uint128_t deferred_id = (uint128_t(to.value) << 64) |
                              current_time_point_sec().sec_since_epoch();
      cancel_deferred(deferred_id);
      t.send(deferred_id, self, true);
    }

    // INLINE_ACTION_SENDER(eosio::token, transfer)(token_account, {{from,
    // active_permission}, {to, active_permission}},{from, to, quantity, memo});
  }
  static std::vector<std::string> get_parameters(const std::string &source) {
    std::vector<std::string> results;
    const std::string delimiter = ",";
    size_t prev = 0;
    size_t next = 0;

    while ((next = source.find_first_of(delimiter.c_str(), prev)) !=
           std::string::npos) {
      if (next - prev != 0) {
        results.push_back(source.substr(prev, next - prev));
      }
      prev = next + 1;
    }

    if (prev < source.size()) {
      results.push_back(source.substr(prev));
    }

    return results;
  }

  static uint64_t convert_to_int(const std::string &parameter) {
    bool isOK = false;
    const char *nptr = parameter.c_str();
    char *endptr = NULL;
    errno = 0;
    uint64_t val = std::strtoull(nptr, &endptr, 10);
    // error ocur
    if ((errno == ERANGE && (val == ULLONG_MAX)) || (errno != 0 && val == 0)) {

    }
    // no digit find
    else if (endptr == nptr) {

    } else if (*endptr != '\0') {
      // printf("Further characters after number: %s\n", endptr);
    } else {
      isOK = true;
    }

    return val;
  }
};