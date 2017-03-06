#include "j2_assertions.h"

typedef TestScope FromJavaScript;

TEST_F(FromJavaScript, Bool) {
  EXPECT_JULIA_JAVASCRIPT_EQ("true", "true");
  EXPECT_JULIA_JAVASCRIPT_NE("false", "true");
  EXPECT_JULIA_JAVASCRIPT_NE("true", "false");
  EXPECT_JULIA_JAVASCRIPT_EQ("false", "false");
}

TEST_F(FromJavaScript, Number) {
  EXPECT_JULIA_JAVASCRIPT_EQ("0.5", "0.5");
  EXPECT_JULIA_JAVASCRIPT_EQ("1.0", "1.0");
  EXPECT_JULIA_JAVASCRIPT_NE("1.0", "1.5");
}

TEST_F(FromJavaScript, String) {
  EXPECT_JULIA_JAVASCRIPT_EQ("\"Hello, world!\"", "\"Hello, world!\"");
  EXPECT_JULIA_JAVASCRIPT_NE("\"Hello, world!\"", "\"Hello, wo\"");
}

TEST_F(FromJavaScript, Array) {
  // ...
}

typedef TestScope FromJulia;

TEST_F(FromJulia, Bool) {
  EXPECT_JS_STRICT_EQ(JavaScriptEval("true"),
                      j2::FromJuliaValue(GetIsolate(), JuliaEval("true")));
  EXPECT_JS_STRICT_EQ(JavaScriptEval("false"),
                      j2::FromJuliaValue(GetIsolate(), JuliaEval("false")));
}

TEST_F(FromJulia, Double) {
  EXPECT_JS_STRICT_EQ(JavaScriptEval("0.5"),
                      j2::FromJuliaValue(GetIsolate(), JuliaEval("0.5")));
  EXPECT_JS_STRICT_EQ(JavaScriptEval("0.5"),
                      j2::FromJuliaValue(GetIsolate(), JuliaEval("0.5")));
}

TEST_F(FromJulia, Nothing) {
  EXPECT_JS_STRICT_EQ(JavaScriptEval("null"),
                      j2::FromJuliaValue(GetIsolate(), JuliaEval("nothing")));
  EXPECT_JS_STRICT_NE(JavaScriptEval("null"),
                      j2::FromJuliaValue(GetIsolate(), JuliaEval("false")));
}

TEST_F(FromJulia, String) {
  EXPECT_JS_STRICT_EQ(
      JavaScriptEval("\"Hello, world!\""),
      j2::FromJuliaValue(GetIsolate(), JuliaEval("\"Hello, world!\"")));
  EXPECT_JS_STRICT_EQ(JavaScriptEval("\"\""),
                      j2::FromJuliaValue(GetIsolate(), JuliaEval("\"\"")));

  EXPECT_JS_STRICT_NE(JavaScriptEval("\"Hello, world!\""),
                      j2::FromJuliaValue(GetIsolate(), JuliaEval("")));
}

//TEST_F(FromJulia, Type) {
//JuliaEval("type T end");
 //}

// TEST_F(Julia, Tuple) {
//  EXPECT_JS_STRICT_EQ(
//    JavaScriptEval("[0, 1, 2, 3, 4]"),
//  j2::FromJuliaValue(GetIsolate(), JuliaEval("[0, 1, 2, 3, 4]")));

//  std::cout << Stringify(JavaScriptEval("({x: 1, y: 2})")) << std::endl;

// ...
//}

/*
TEST_F(Julia, Array) {
  EXPECT_JS_STRICT_EQ(
      JavaScriptEval("[0, 1, 2, 3, 4]"),
      j2::NewFromJuliaValue(GetIsolate(), JuliaEval("[0, 1, 2, 3, 4]")));
}
*/
