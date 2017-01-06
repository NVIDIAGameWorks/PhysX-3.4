/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_TEST_H
#define APEX_TEST_H

#define WARNING(exp, msg) if (!(exp)) {ret &= (exp); APEX_DEBUG_WARNING(msg);}
#define EXPECT_TRUE(exp) WARNING(exp, "Expected true: " #exp)
#define EXPECT_EQ(v1, v2) WARNING(v1 == v2, "Expected: " #v1 " == " #v2)
#define EXPECT_NE(v1, v2) WARNING(v1 != v2, "Expected: " #v1 " != " #v2)
#define EXPECT_GE(v1, v2) WARNING(v1 >= v2, "Expected: " #v1 " >= " #v2)
#define EXPECT_GT(v1, v2) WARNING(v1 > v2, "Expected: " #v1 " > " #v2)
#define EXPECT_LE(v1, v2) WARNING(v1 <= v2, "Expected: " #v1 " <= " #v2)
#define EXPECT_LT(v1, v2) WARNING(v1 < v2, "Expected: " #v1 " < " #v2)

#endif //APEX_TEST_H
