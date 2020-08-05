// =============================================================================
// Copyright 2017 National Technology & Engineering Solutions of Sandia, LLC
// (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// =============================================================================
// Maintained by Sandia National Laboratories <Netmeld@sandia.gov>
// =============================================================================

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include <netmeld/datastore/parsers/ParserTestHelper.hpp>

#include "Parser.hpp"

namespace nmdp = netmeld::datastore::parsers;

using qi::ascii::blank;

class TestUbdpParser : public Parser<nmdp::IstreamIter> {
  public:
    using Parser::start;

    DataContainerSingleton& dcs {DataContainerSingleton::getInstance()};
};

BOOST_AUTO_TEST_CASE(testUbdp)
{
  TestUbdpParser tp;
  const auto& parserRule {tp};

  std::vector<std::string> testsOk {
R"STR([
  {
        "ubdp": {
          "ubdp.preamble": "0x00000100",
          "ubdp.size": "138",
          "MAC and IP Address": {
            "ubdp.type": "2",
            "ubdp.len": "10",
            "ubdp.mac": "00:11:22:33:44:55",
            "ubdp.ip": "4.3.2.1"
          },
          "MAC and IP Address": {
            "ubdp.type": "2",
            "ubdp.len": "10",
            "ubdp.mac": "11:22:33:44:55:66",
            "ubdp.ip": "8.4.2.1"
          },
          "MAC and IP Address": {
            "ubdp.type": "2",
            "ubdp.len": "10",
            "ubdp.mac": "12:34:12:34:12:34",
            "ubdp.ip": "1.2.3.4"
          },
          "MAC Address": {
            "ubdp.type": "1",
            "ubdp.len": "6",
            "ubdp.mac": "11:22:33:44:55:66"
          },
          "Uptime": {
            "ubdp.type": "10",
            "ubdp.len": "4",
            "ubdp.uptime": "12345"
          },
          "Hostname": {
            "ubdp.type": "11",
            "ubdp.len": "5",
            "ubdp.hostname": "SomeHostname"
          },
          "Product": {
            "ubdp.type": "12",
            "ubdp.len": "8",
            "ubdp.product": "Some-Product-ID"
          },
          "Firmware Detailed": {
            "ubdp.type": "3",
            "ubdp.len": "54",
            "ubdp.firmware_full": "Device.FM.v1.2.3-hotfix.1234"
          },
          "Unknown type": {
            "ubdp.type": "24",
            "ubdp.len": "4",
            "ubdp.unk": "00:00:00:00"
          }
        }
      }
    }
  }
]
)STR",
  };

  for (const auto& test : testsOk) {
    BOOST_TEST(nmdp::test(test.c_str(), parserRule, blank, false));
    auto out {tp.dcs.getData()};
    BOOST_TEST(1 == out.size());

    auto ifaces {out[0].ifaces};
    BOOST_TEST(1 == ifaces.size());

    auto macAddrs {out[0].macAddrs};
    BOOST_TEST(3 == macAddrs.size());
    std::vector<std::tuple<std::string, std::string>> macValues {
      {"00:11:22:33:44:55","4.3.2.1/32"},
      {"11:22:33:44:55:66","8.4.2.1/32"},
      {"12:34:12:34:12:34","1.2.3.4/32"},
    };
    auto macValuesIter {macValues.begin()};
    for (const auto& [id, macAddr] : macAddrs) {
      //LOG_INFO << "macAddr: " << macAddr << '\n';
      std::string tgtMac, tgtIp;
      std::tie(tgtMac, tgtIp) = *macValuesIter;
      ++macValuesIter;
      BOOST_TEST(tgtMac ==  macAddr.toString());
      auto ipAddrs {macAddr.getIpAddrs()};
      BOOST_TEST(1 == ipAddrs.size());
      BOOST_TEST(tgtIp == ipAddrs[0].toString());
    }
  }
}
