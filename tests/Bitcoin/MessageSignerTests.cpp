// Copyright © 2017-2022 Trust Wallet.
//
// This file is part of Trust. The full Trust copyright notice, including
// terms governing use, modification, and redistribution, is contained in the
// file LICENSE at the root of the source code distribution tree.

#include "Bitcoin/MessageSigner.h"
#include "Bitcoin/Address.h"
#include "HexCoding.h"
#include "PrivateKey.h"
#include "PublicKey.h"
#include "Base64.h"
#include "Coin.h"
#include "Data.h"

#include <TrustWalletCore/TWCoinType.h>
#include <TrustWalletCore/TWPublicKeyType.h>

#include <cassert>
#include <gtest/gtest.h>
#include <iostream> // TODO remove

namespace TW::Bitcoin::MessageSignerTests {

const auto privateKey = PrivateKey(parse_hex("afeefca74d9a325cf1d6b6911d61a65c32afa8e02bd5e78e2e4ac2910bab45f5"));

TEST(BitcoinMessageSigner, VerifyMessage) {
    EXPECT_TRUE(MessageSigner::verifyMessage(
        "1B8Qea79tsxmn4dTiKKRVvsJpHwL2fMQnr",
        "test signature",
        "H+3L5IbSVcejp4S2VwLXCxLEMQAWDvKbE8lQyq0ocdvyM1aoEudkzN/S/qLI3vnNOFY6V13BXWSFrPr3OjGa5Dk="
    ));
    EXPECT_TRUE(MessageSigner::verifyMessage(
        "1HZwkjkeaoZfTSaJxDw6aKkxp45agDiEzN",
        "This is an example of a signed message.",
        "G39Qf0XrZHICWbz3r5gOkcgTRw3vM4leGjiR3refr/K1OezcKmmXaLn4zc8ji2rjbBUIMrIhH/jc5Z2qEEz7qVk="
    ));
    EXPECT_TRUE(MessageSigner::verifyMessage(
        "1H8X4u6CVZRTLLNbUQTKAnc5vCkqWMpwfF",
        "compressed key",
        "IKUI9v2xbHogJe8HKXI2M5KEhMKaW6fjNxtyEy27Mf+3/e1ht4jZoc85e4F8stPsxt4Xcg8Yr42S28O6L/Qx9fE="
    ));
    EXPECT_TRUE(MessageSigner::verifyMessage(
        "19cAJn4Ms8jodBBGtroBNNpCZiHAWGAq7X",
        "test signature",
        "ILH5K7JQLaRGaKGXXH5mYM6FIIy9IWyY4JUPI+PHYY4WaupxUbg+zy0bhBCrDuehy9x4WidwjkRR1GSLnWvOXBo="
    ));
    EXPECT_TRUE(MessageSigner::verifyMessage(
        "19cAJn4Ms8jodBBGtroBNNpCZiHAWGAq7X",
        "another text",
        "H7vrF2C+TlFiHyegAw3QLv6SK0myuEEXUOgfx0+Qio1YVDuSa6p/OHpoQVlUt3F8QJdbdZN9M1h/fYEAnEz16V0="
    ));
    EXPECT_TRUE(MessageSigner::verifyMessage(
        "1E4T9JZ3mq6cdgiRJEWzHqDXb9t322fE6d",
        "test signature",
        "HLH5K7JQLaRGaKGXXH5mYM6FIIy9IWyY4JUPI+PHYY4WaupxUbg+zy0bhBCrDuehy9x4WidwjkRR1GSLnWvOXBo="
    ));
}

TEST(BitcoinMessageSigner, SignAndVerify) {
    const auto pubKey = privateKey.getPublicKey(TWPublicKeyTypeSECP256k1);
    EXPECT_EQ(hex(pubKey.bytes), "0399c6f51ad6f98c9c583f8e92bb7758ab2ca9a04110c0a1126ec43e5453d196c1");
    const auto address = Address(pubKey, TW::p2pkhPrefix(TWCoinTypeBitcoin)).string();
    EXPECT_EQ(address, "19cAJn4Ms8jodBBGtroBNNpCZiHAWGAq7X");

    {
        const auto msg = "test signature";
        const auto signature = MessageSigner::signMessage(privateKey, address, msg);
        EXPECT_EQ(signature, "ILH5K7JQLaRGaKGXXH5mYM6FIIy9IWyY4JUPI+PHYY4WaupxUbg+zy0bhBCrDuehy9x4WidwjkRR1GSLnWvOXBo=");

        EXPECT_TRUE(MessageSigner::verifyMessage(address, msg, signature));
    }
    {
        const auto msg = "another text";
        const auto signature = MessageSigner::signMessage(privateKey, address, msg);
        EXPECT_EQ(signature, "H7vrF2C+TlFiHyegAw3QLv6SK0myuEEXUOgfx0+Qio1YVDuSa6p/OHpoQVlUt3F8QJdbdZN9M1h/fYEAnEz16V0=");

        EXPECT_TRUE(MessageSigner::verifyMessage(address, msg, signature));
    }

    // uncompressed
    const auto pubKeyUncomp = privateKey.getPublicKey(TWPublicKeyTypeSECP256k1Extended);
    const auto keyHash = pubKeyUncomp.hash(Data{TW::p2pkhPrefix(TWCoinTypeBitcoin)}, Hash::HasherSha256ripemd);
    const auto addressUncomp = Address(keyHash).string();
    EXPECT_EQ(addressUncomp, "1E4T9JZ3mq6cdgiRJEWzHqDXb9t322fE6d");
    {
        const auto msg = "test signature";
        const auto signature = MessageSigner::signMessage(privateKey, addressUncomp, msg, false);
        EXPECT_EQ(signature, "HLH5K7JQLaRGaKGXXH5mYM6FIIy9IWyY4JUPI+PHYY4WaupxUbg+zy0bhBCrDuehy9x4WidwjkRR1GSLnWvOXBo=");

        EXPECT_TRUE(MessageSigner::verifyMessage(addressUncomp, msg, signature));
    }
}

TEST(BitcoinMessageSigner, SignNegative) {
    const auto address = Address(privateKey.getPublicKey(TWPublicKeyTypeSECP256k1), TW::p2pkhPrefix(TWCoinTypeBitcoin)).string();
    EXPECT_EQ(address, "19cAJn4Ms8jodBBGtroBNNpCZiHAWGAq7X");
    const auto msg = "test signature";
    // Use invalid address
    EXPECT_EXCEPTION(MessageSigner::signMessage(privateKey, "__THIS_IS_NOT_A_VALID_ADDRESS__", msg), "Address is not valid (legacy) address");
    // Use invalid address, not legacy
    EXPECT_EXCEPTION(MessageSigner::signMessage(privateKey, "bc1qpjult34k9spjfym8hss2jrwjgf0xjf40ze0pp8", msg), "Address is not valid (legacy) address");
    // Use valid, but not matching key
    EXPECT_EXCEPTION(MessageSigner::signMessage(privateKey, "1B8Qea79tsxmn4dTiKKRVvsJpHwL2fMQnr", msg), "Address does not match key");
}

TEST(BitcoinMessageSigner, VerifyNegative) {
    const auto sig = parse_hex("1fedcbe486d255c7a3a784b65702d70b12c43100160ef29b13c950caad2871dbf23356a812e764ccdfd2fea2c8def9cd38563a575dc15d6485acfaf73a319ae439");
    // Baseline positive
    EXPECT_TRUE(MessageSigner::verifyMessage(
        "1B8Qea79tsxmn4dTiKKRVvsJpHwL2fMQnr",
        "test signature",
        sig
    ));

    // Provide non-matching address
    EXPECT_FALSE(MessageSigner::verifyMessage(
        "1HZwkjkeaoZfTSaJxDw6aKkxp45agDiEzN",
        "test signature",
        sig
    ));
    // Signature too short
    EXPECT_EXCEPTION(MessageSigner::verifyMessage(
        "1HZwkjkeaoZfTSaJxDw6aKkxp45agDiEzN",
        "test signature",
        parse_hex("1fedcbe486d255c7a3a784b65702d70b12c43100160ef29b13c950caad2871dbf23356a812e764ccdfd2fea2c8def9cd38563a575dc15d6485acfaf73a319ae4")
    ), "signature too short");
    // Invalid address  
    EXPECT_EXCEPTION(MessageSigner::verifyMessage(
        "__THIS_IS_NOT_A_VALID_ADDRESS__",
        "test signature",
        parse_hex("1fedcbe486d255c7a3a784b65702d70b12c43100160ef29b13c950caad2871dbf23356a812e764ccdfd2fea2c8def9cd38563a575dc15d6485acfaf73a319ae4")
    ), "Input address invalid");
    EXPECT_EXCEPTION(MessageSigner::verifyMessage(
        "bc1qpjult34k9spjfym8hss2jrwjgf0xjf40ze0pp8",
        "test signature",
        parse_hex("1fedcbe486d255c7a3a784b65702d70b12c43100160ef29b13c950caad2871dbf23356a812e764ccdfd2fea2c8def9cd38563a575dc15d6485acfaf73a319ae4")
    ), "Input address invalid");
}

} // namespace TW::Bitcoin
