#ifndef BEE_SCANNER_TEST
#define BEE_SCANNER_TEST

#include "scanner.hpp"
#include <gtest/gtest.h>
#include <string>

namespace bee
{

constexpr std::string_view bee_snake_source =
    R"(
import_c "SDL2/SDL.h"
mem    :: import "std/mem.bee"
random :: import "std/random.bee"
error  :: import "std/error.bee"

Snake :: struct
{
    x: s32
    y: s32
    next: &Snake

    new :: (x: s32, y: s32) -> &Snake
    {
        return mem.new(Snake{x, y, none})
    }

    free :: (&Snake)
    {
        .next.free()
        mem.free(.next)
    }

    grow :: (&Snake)
    {
        if .next != none
        {
            return .next.grow()
        }
        .next = Snake.new(x, y)
    }

    move :: (&Snake, x: s32, y: s32)
    {
        if .next != none
        {
            .next.move(.x, .y)
        }
        .x, .y = x, y
    }

    draw :: (&Snake, rnd :: &SDL_Renderer, square_w: s32, square_h: s32)
    {
        if .next != none
        {
            return .next.draw(rnd, square_w, square_h)
        }
        SDL_SetRenderDrawColor(rnd, 255, 255, 255, 255)
        SDL_RenderDrawRect(rnd, &SDL_Rect{x * square_w, y * square_h, square_w, square_h})
    }
}

Food :: struct
{
    x: s32
    y: s32

    random :: (board_w: s32, board_h: s32) -> Food
    {
        return Food
        {
            x: random.ranged(0..board_w),
            y: random.ranged(0..board_h),
        }
    }

    draw :: (&Food, rnd: &SDL_Renderer, square_w: s32, square_h: s32)
    {
        SDL_SetRenderDrawColor(rnd, 16, 0, 240, 255)
        SDL_RenderDrawRect(rnd, &SDL_Rect{x * square_w, y * square_h, square_w, square_h})
    }
}

Snake_Game :: struct
{
    snake: &Snake
    wnd: &SDL_Window
    rnd: &SDL_Renderer
    keyboard: &[u8]
    wnd_w: u32
    wnd_h: u32
    board_w: s32
    board_h: s32
    score: s64

    new :: (wnd_w: u32, wnd_h: u32, board_w: s32, board_h: s32) -> Game, Error
    {
        on_context_error :: () -> Game, Error
        {
            return Game{}, error.newf("SDL :: {:s}", SDL_GetError())
        }

        if SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0
        {
            return on_context_error()
        }

        game := Game{}

        if game.wnd = SDL_CreateWindow
        (
            "Beesnake",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            wnd_w,
            wnd_h,
            0
        ); game.wnd != some
        {
            return on_context_error()
        }

        if game.rnd = SDL_CreateRenderer(game.wnd, -1, 0); game.rnd != some
        {
            return on_context_error()
        }

        keycount := 0
        keystate := SDL_GetKeyboardState(&keycount)
        if !keystate
        {
            return on_context_error()
        } 
        
        game.keystate = &[u8](keystate, keycount)
        game.snake = Snake.new(board_w / 2, board_h / 2)
        game.food = Food.new(board_w, board_h)
        game.board_w, game.board_h = board_w, board_h
        game.wnd_w, game.wnd_h = wnd_w, wnd_h

        return game. error.Ok
    }

    update :: (&Game)
    {
        // TODO! 
    }
}

main :: () -> u32
{
    return 0
}
)";

static testing::AssertionResult assert_tokens(std::string_view _src, std::vector<Token> tokens)
{
    std::string src = std::string{_src} + "\n";
    Scanner scanner{src, bee_syntax_map()};

    for (const Token &expected : tokens)
    {
        Token result = scanner.tokenize();

        if (result.expr != expected.expr || result.type != expected.type)
        {
            Error error = bee_errorf("scanner test error", src, result, "'{}' :: '{}' != '{}' :: '{}'", result.expr,
                                     token_typename(result.type), expected.expr, token_typename(expected.type));

            return testing::AssertionFailure() << error.what();
        }
    }

    return testing::AssertionSuccess();
}

#define EXPECT_SCAN(src, ...) EXPECT_TRUE(assert_tokens(src, {__VA_ARGS__}))

TEST(Scanner, Syntax_Map)
{
    Syntax_Map map = bee_syntax_map();
}

TEST(Scanner, Comments)
{
    EXPECT_SCAN("// Hello, World !\n", {"// Hello, World !", Token_Comment});

    EXPECT_SCAN(R"(
// Comment is simple with bee
main :: () -> s32
// Just put two dashes and end with a newline
{
}
)",

                {"// Comment is simple with bee", Token_Comment}, {"main", Token_Id}, {"::", Token_Declare},
                {"(", Token_Parent_Begin}, {")", Token_Parent_End}, {"->", Token_Arrow}, {"s32", Token_Id},
                {"// Just put two dashes and end with a newline", Token_Comment}, {"{", Token_Scope_Begin},
                {"}", Token_Scope_End});
}

TEST(Scanner, Keyword)
{
    EXPECT_SCAN("struct", {"struct", Token_Struct});
    EXPECT_SCAN("enum", {"enum", Token_Enum});
    EXPECT_SCAN("union", {"union", Token_Union});
    EXPECT_SCAN("break", {"break", Token_Break});
    EXPECT_SCAN("case", {"case", Token_Case});
    EXPECT_SCAN("continue", {"continue", Token_Continue});
    EXPECT_SCAN("else", {"else", Token_Else});
    EXPECT_SCAN("for", {"for", Token_For});
    EXPECT_SCAN("if", {"if", Token_If});
    EXPECT_SCAN("return", {"return", Token_Return});
    EXPECT_SCAN("switch", {"switch", Token_Switch});
    EXPECT_SCAN("while", {"while", Token_While});
    EXPECT_SCAN("and", {"and", Token_And});
    EXPECT_SCAN("or", {"or", Token_Or});
}

TEST(Lexer, Operator)
{
    EXPECT_SCAN("++", {"++", Token_Increment});
    EXPECT_SCAN("--", {"--", Token_Decrement});
    EXPECT_SCAN("::", {"::", Token_Declare});
    EXPECT_SCAN(":", {":", Token_Define});
    EXPECT_SCAN("->", {"->", Token_Arrow});
    EXPECT_SCAN("{", {"{", Token_Scope_Begin});
    EXPECT_SCAN("}", {"}", Token_Scope_End});
    EXPECT_SCAN("(", {"(", Token_Parent_Begin});
    EXPECT_SCAN(")", {")", Token_Parent_End});
    EXPECT_SCAN("]", {"]", Token_Crochet_Begin});
    EXPECT_SCAN("[", {"[", Token_Crochet_End});
    EXPECT_SCAN(";", {";", Token_Semicolon});
    EXPECT_SCAN(",", {",", Token_Comma});
    EXPECT_SCAN("&&", {"&&", Token_And});
    EXPECT_SCAN("||", {"||", Token_Or});
    EXPECT_SCAN(".", {".", Token_Dot});
    EXPECT_SCAN("~", {"~", Token_Bin_Not});
    EXPECT_SCAN("|", {"|", Token_Bin_Or});
    EXPECT_SCAN("^", {"^", Token_Bin_Xor});
    EXPECT_SCAN("<<", {"<<", Token_Shift_L});
    EXPECT_SCAN(">>", {">>", Token_Shift_R});
    EXPECT_SCAN("+", {"+", Token_Add});
    EXPECT_SCAN("-", {"-", Token_Sub});
    EXPECT_SCAN("/", {"/", Token_Div});
    EXPECT_SCAN("%", {"%", Token_Mod});
    EXPECT_SCAN("==", {"==", Token_Eq});
    EXPECT_SCAN("!=", {"!=", Token_Not_Eq});
    EXPECT_SCAN("<=", {"<=", Token_Less_Eq});
    EXPECT_SCAN(">=", {">=", Token_Greater_Eq});
    EXPECT_SCAN("<", {"<", Token_Less});
    EXPECT_SCAN(">", {">", Token_Greater});
    EXPECT_SCAN("!", {"!", Token_Not});
    EXPECT_SCAN("=", {"=", Token_Assign});
    EXPECT_SCAN("&", {"&", Token_Ref});

    // Operators does not require spaces to match !
    EXPECT_SCAN("!bee", {"!", Token_Not}, {"bee", Token_Id});
    EXPECT_SCAN("! bee", {"!", Token_Not}, {"bee", Token_Id});

    EXPECT_SCAN("bee++", {"bee", Token_Id}, {"++", Token_Increment});
    EXPECT_SCAN("++bee", {"++", Token_Increment}, {"bee", Token_Id});
    EXPECT_SCAN("bee ++", {"bee", Token_Id}, {"++", Token_Increment});
    EXPECT_SCAN("++ bee", {"++", Token_Increment}, {"bee", Token_Id});
    EXPECT_SCAN("+ + ", {"+", Token_Add}, {"+", Token_Add});
    EXPECT_SCAN("+++ ", {"++", Token_Increment}, {"+", Token_Add});
    EXPECT_SCAN("+ ++", {"+", Token_Add}, {"++", Token_Increment});
    EXPECT_SCAN("++ +", {"++", Token_Increment}, {"+", Token_Add});

    EXPECT_SCAN("bee.member", {"bee", Token_Id}, {".", Token_Dot}, {"member", Token_Id});
}

TEST(Scanner, Int)
{
    EXPECT_SCAN("1234567890", {"1234567890", Token_Int_Dec});
    EXPECT_SCAN("0", {"0", Token_Int_Dec});
    EXPECT_SCAN("-0", {"-", Token_Sub}, {"0", Token_Int_Dec});
    EXPECT_SCAN("+0", {"+", Token_Add}, {"0", Token_Int_Dec});
    EXPECT_SCAN("+2147483647", {"+", Token_Add}, {"2147483647", Token_Int_Dec});
    EXPECT_SCAN("-2147483648", {"-", Token_Sub}, {"2147483648", Token_Int_Dec});
    EXPECT_SCAN("-+1", {"-", Token_Sub}, {"+", Token_Add}, {"1", Token_Int_Dec});
    EXPECT_SCAN("+-1", {"+", Token_Add}, {"-", Token_Sub}, {"1", Token_Int_Dec});
    EXPECT_SCAN("++1", {"++", Token_Increment}, {"1", Token_Int_Dec});

    EXPECT_SCAN("0b0", {"0b0", Token_Int_Bin});
    EXPECT_SCAN("0b1", {"0b1", Token_Int_Bin});
    EXPECT_SCAN("0b1011101", {"0b1011101", Token_Int_Bin});

    EXPECT_SCAN("0x0", {"0x0", Token_Int_Hex});
    EXPECT_SCAN("0xf", {"0xf", Token_Int_Hex});
    EXPECT_SCAN("0xabcdef", {"0xabcdef", Token_Int_Hex});
}

TEST(Scanner, Float)
{
    EXPECT_SCAN("0.0", {"0.0", Token_Float});
    EXPECT_SCAN(".0", {".0", Token_Float});
    EXPECT_SCAN("0.", {"0.", Token_Float});

    EXPECT_SCAN("1234567890.0987654321", {"1234567890.0987654321", Token_Float});
    EXPECT_SCAN(".0987654321", {".0987654321", Token_Float});
    EXPECT_SCAN("1234567890.", {"1234567890.", Token_Float});
}

TEST(Scanner, String)
{
    EXPECT_SCAN(R"( "" // Empty String Literal )", {"\"\"", Token_Str});
    EXPECT_SCAN(R"( "Hello, World!" )", {R"("Hello, World!")", Token_Str});

    EXPECT_SCAN(R"( "\"" )", {R"("\"")", Token_Str});
    EXPECT_SCAN(R"( "\"Hello\" World" )", {R"("\"Hello\" World")", Token_Str});

    // Escape sequences
    EXPECT_SCAN(R"("\'")", {R"("\'")", Token_Str});
    EXPECT_SCAN(R"("\"")", {R"("\"")", Token_Str});
    EXPECT_SCAN(R"("\?")", {R"("\?")", Token_Str});
    EXPECT_SCAN(R"("\\")", {R"("\\")", Token_Str});
    EXPECT_SCAN(R"("\a")", {R"("\a")", Token_Str});
    EXPECT_SCAN(R"("\b")", {R"("\b")", Token_Str});
    EXPECT_SCAN(R"("\f")", {R"("\f")", Token_Str});
    EXPECT_SCAN(R"("\n")", {R"("\n")", Token_Str});
    EXPECT_SCAN(R"("\r")", {R"("\r")", Token_Str});
    EXPECT_SCAN(R"("\t")", {R"("\t")", Token_Str});
    EXPECT_SCAN(R"("\v")", {R"("\v")", Token_Str});
}

TEST(Scanner, Char)
{
    EXPECT_SCAN("'c'", {"'c'", Token_Char});

    EXPECT_SCAN(R"('\'')", {R"('\'')", Token_Char});
    EXPECT_SCAN(R"('\"')", {R"('\"')", Token_Char});
    EXPECT_SCAN(R"('\?')", {R"('\?')", Token_Char});
    EXPECT_SCAN(R"('\\')", {R"('\\')", Token_Char});
    EXPECT_SCAN(R"('\a')", {R"('\a')", Token_Char});
    EXPECT_SCAN(R"('\b')", {R"('\b')", Token_Char});
    EXPECT_SCAN(R"('\f')", {R"('\f')", Token_Char});
    EXPECT_SCAN(R"('\n')", {R"('\n')", Token_Char});
    EXPECT_SCAN(R"('\r')", {R"('\r')", Token_Char});
    EXPECT_SCAN(R"('\t')", {R"('\t')", Token_Char});
    EXPECT_SCAN(R"('\v')", {R"('\v')", Token_Char});
}

TEST(Scanner, Min)
{
    constexpr std::string_view source =
        R"(
// Bee minimal source code
main :: () -> s32
{
    return 0
}
)";

    EXPECT_SCAN(source, {"// Bee minimal source code", Token_Comment}, {"main", Token_Id}, {"::", Token_Declare},
                {"(", Token_Parent_Begin}, {")", Token_Parent_End}, {"->", Token_Arrow}, {"s32", Token_Id},
                {"{", Token_Scope_Begin}, {"return", Token_Return}, {"0", Token_Int_Dec}, {"}", Token_Scope_End});
}

TEST(Scanner, Fib)
{
    constexpr std::string_view source =
        R"(
fib :: (n: s64) -> s64
{
    if n == 0 or n == 1
    {
        return n
    }
    else
    {
        return fib(n - 1) + fib(n - 2)
    }
}
)";

    EXPECT_SCAN(source, {"fib", Token_Id}, {"::", Token_Declare}, {"(", Token_Parent_Begin}, {"n", Token_Id},
                {":", Token_Define}, {"s64", Token_Id}, {")", Token_Parent_End}, {"->", Token_Arrow}, {"s64", Token_Id},
                {"{", Token_Scope_Begin}, {"if", Token_If}, {"n", Token_Id}, {"==", Token_Eq}, {"0", Token_Int_Dec},
                {"or", Token_Or}, {"n", Token_Id}, {"==", Token_Eq}, {"1", Token_Int_Dec}, {"{", Token_Scope_Begin},
                {"return", Token_Return}, {"n", Token_Id}, {"}", Token_Scope_End}, {"else", Token_Else},
                {"{", Token_Scope_Begin}, {"return", Token_Return}, {"fib", Token_Id}, {"(", Token_Parent_Begin},
                {"n", Token_Id}, {"-", Token_Sub}, {"1", Token_Int_Dec}, {")", Token_Parent_End}, {"+", Token_Add},
                {"fib", Token_Id}, {"(", Token_Parent_Begin}, {"n", Token_Id}, {"-", Token_Sub}, {"2", Token_Int_Dec},
                {")", Token_Parent_End}, {"}", Token_Scope_End}, {"}", Token_Scope_End});
}

} // namespace bee

#endif
