#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <cctype>

using namespace std;

// ================= TOKEN TYPES =================

enum class TokenType
{
    // DATA TYPES
    MAMA_NUMBER,
    MAMA_STRING,

    // KEYWORDS
    IF,
    ELSE,
    PRINT,

    // VALUES
    ID,
    NUMBER,
    STRING_LITERAL,

    // OPERATORS
    PLUS,
    MINUS,
    MUL,
    DIV,

    ASSIGN,

    GREATER,
    LESS,
    EQUAL_EQUAL,

    // SYMBOLS
    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,

    SEMI,

    EOF_TOKEN
};

// ======================================================
// VARIABLE TYPES(tarek)
// ======================================================

enum class VarType
{
    INT,
    STRING
};

// ======================================================
// TOKEN
// ======================================================

class Token
{
public:
    TokenType type;
    string lexeme;
    int line;

    Token(TokenType type, string lexeme, int line)
    {
        this->type = type;
        this->lexeme = lexeme;
        this->line = line;
    }

    string toString()
    {
        return lexeme;
    }
};

// ================= AST (turjo) =================

class ASTNode
{
public:
    virtual ~ASTNode() = default;
};

class NumberNode : public ASTNode
{
public:
    int value;

    NumberNode(int value)
    {
        this->value = value;
    }
};

class VarNode : public ASTNode
{
public:
    string name;

    VarNode(string name)
    {
        this->name = name;
    }
};

class BinOpNode : public ASTNode
{
public:
    ASTNode *left;
    string op;
    ASTNode *right;

    BinOpNode(ASTNode *left, string op, ASTNode *right)
    {
        this->left = left;
        this->op = op;
        this->right = right;
    }
};

class AssignNode : public ASTNode
{
public:
    string name;
    ASTNode *expr;

    AssignNode(string name, ASTNode *expr)
    {
        this->name = name;
        this->expr = expr;
    }
};

// ================= SYMBOL TABLE (turjo) =================

class SymbolTable
{
private:
    unordered_map<string, int> table;

public:
    void set(string name, int value)
    {
        table[name] = value;
    }

    bool contains(string name)
    {
        return table.find(name) != table.end();
    }

    int get(string name)
    {
        return table[name];
    }

    void display(ofstream &out)
    {
        for (auto x : table)
        {
            out << x.first << " = " << x.second << "\n";
        }
    }
};

// ================= SEMANTIC ANALYZER (istiak jubayer)=================

class SemanticAnalyzer
{
private:
    SymbolTable &symbolTable;

public:
    SemanticAnalyzer(SymbolTable &st)
        : symbolTable(st) {}

    void analyze(vector<ASTNode *> &nodes)
    {
        for (auto node : nodes)
        {
            evaluate(node);
        }
    }

    int evaluate(ASTNode *node)
    {

        if (auto n = dynamic_cast<NumberNode *>(node))
        {
            return n->value;
        }

        if (auto v = dynamic_cast<VarNode *>(node))
        {

            if (!symbolTable.contains(v->name))
            {
                throw runtime_error(
                    "Undefined variable: " + v->name);
            }

            return symbolTable.get(v->name);
        }

        if (auto b = dynamic_cast<BinOpNode *>(node))
        {

            int left = evaluate(b->left);
            int right = evaluate(b->right);

            if (b->op == "+")
                return left + right;

            if (b->op == "-")
                return left - right;

            if (b->op == "*")
                return left * right;

            if (b->op == "/")
            {
                if (right == 0)
                {
                    throw runtime_error(
                        "Division by zero");
                }

                return left / right;
            }
        }

        if (auto a = dynamic_cast<AssignNode *>(node))
        {

            int value = evaluate(a->expr);

            symbolTable.set(a->name, value);

            return value;
        }

        return 0;
    }

    static int convertBanglaNumber(string s)
    {
        vector<string> bn = {
            "০", "১", "২", "৩", "৪",
            "৫", "৬", "৭", "৮", "৯"};

        string result = "";

        for (int i = 0; i < s.size();)
        {

            bool found = false;

            for (int d = 0; d <= 9; d++)
            {

                if (s.substr(i, bn[d].size()) == bn[d])
                {
                    result += char('0' + d);
                    i += bn[d].size();
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                i++;
            }
        }

        if (result.empty())
            return 0;

        return stoi(result);
    }
};

// ================= LEXER =================

class Lexer
{
private:
    string source;
    vector<Token> tokens;

    int start = 0;
    int current = 0;
    int line = 1;

    unordered_map<string, TokenType> keywords = {
        {"মামা", TokenType::MAMA}};

public:
    Lexer(string source)
    {
        this->source = source;
    }

    vector<Token> scanTokens()
    {

        while (!isAtEnd())
        {
            start = current;
            scanToken();
        }

        tokens.push_back(
            Token(TokenType::EOF_TOKEN, "", line));

        return tokens;
    }

private:
    bool isBanglaDigitStart(unsigned char c)
    {
        return c >= 224;
    }

    void scanToken()
    {

        unsigned char c = advance();

        switch (c)
        {
        case '(':
            addToken(TokenType::LPAREN);
            break;

        case ')':
            addToken(TokenType::RPAREN);
            break;

        case '+':
            addToken(TokenType::PLUS);
            break;

        case '-':
            addToken(TokenType::MINUS);
            break;

        case '*':
            addToken(TokenType::MUL);
            break;

        case '/':
            addToken(TokenType::DIV);
            break;

        case '=':
            addToken(TokenType::ASSIGN);
            break;

        case ';':
            addToken(TokenType::SEMI);
            break;

        case ' ':
        case '\r':
        case '\t':
            break;

        case '\n':
            line++;
            break;

        default:

            // English number
            if (isdigit(c))
            {
                number();
            }

            // Bangla UTF-8 sequence
            else if (isBanglaDigitStart(c))
            {

                string temp;
                temp += c;

                if (current < source.size())
                    temp += source[current];

                if (current + 1 < source.size())
                    temp += source[current + 1];

                vector<string> bn = {
                    "০", "১", "২", "৩", "৪",
                    "৫", "৬", "৭", "৮", "৯"};

                bool isNum = false;

                for (auto x : bn)
                {
                    if (temp == x)
                    {
                        isNum = true;
                        break;
                    }
                }

                if (isNum)
                    number();
                else
                    identifier();
            }

            else if (isalpha(c))
            {
                identifier();
            }

            break;
        }
    }

    void identifier()
    {

        while (
            isalnum(peek()) ||
            (unsigned char)peek() >= 128)
        {
            advance();
        }

        string text =
            source.substr(start, current - start);

        if (keywords.find(text) != keywords.end())
        {
            addToken(keywords[text]);
        }
        else
        {
            addToken(TokenType::ID);
        }
    }

    void number()
    {

        while (
            isdigit(peek()) ||
            (unsigned char)peek() >= 128)
        {
            advance();
        }

        addToken(TokenType::NUMBER);
    }

    char advance()
    {
        return source[current++];
    }

    char peek()
    {
        if (isAtEnd())
            return '\0';

        return source[current];
    }

    bool isAtEnd()
    {
        return current >= source.size();
    }

    void addToken(TokenType type)
    {
        tokens.push_back(
            Token(
                type,
                source.substr(start, current - start),
                line));
    }
};

// ================= PARSER =================

class Parser
{
private:
    vector<Token> tokens;
    int current = 0;

public:
    Parser(vector<Token> tokens)
    {
        this->tokens = tokens;
    }

    vector<ASTNode *> parse()
    {

        vector<ASTNode *> nodes;

        while (!isAtEnd())
        {
            nodes.push_back(statement());
        }

        return nodes;
    }

private:
    ASTNode *statement()
    {

        if (match({TokenType::MAMA}))
        {
            return declaration();
        }

        throw runtime_error(
            "Expected variable declaration");
    }

    ASTNode *declaration()
    {

        Token name = consume(
            TokenType::ID,
            "Expected variable name");

        consume(
            TokenType::ASSIGN,
            "Expected '='");

        ASTNode *expr = expression();

        consume(
            TokenType::SEMI,
            "Expected ';'");

        return new AssignNode(
            name.lexeme,
            expr);
    }

    ASTNode *expression()
    {

        ASTNode *node = term();

        while (
            match({TokenType::PLUS,
                   TokenType::MINUS}))
        {

            string op = previous().lexeme;

            node = new BinOpNode(
                node,
                op,
                term());
        }

        return node;
    }

    ASTNode *term()
    {

        ASTNode *node = primary();

        while (
            match({TokenType::MUL,
                   TokenType::DIV}))
        {

            string op = previous().lexeme;

            node = new BinOpNode(
                node,
                op,
                primary());
        }

        return node;
    }

    ASTNode *primary()
    {

        if (match({TokenType::NUMBER}))
        {

            return new NumberNode(
                SemanticAnalyzer::
                    convertBanglaNumber(
                        previous().lexeme));
        }

        if (match({TokenType::ID}))
        {
            return new VarNode(
                previous().lexeme);
        }

        if (match({TokenType::LPAREN}))
        {

            ASTNode *expr = expression();

            consume(
                TokenType::RPAREN,
                "Expected ')'");

            return expr;
        }

        throw runtime_error(
            "Expected expression");
    }

    bool match(initializer_list<TokenType> types)
    {

        for (TokenType type : types)
        {

            if (check(type))
            {
                advance();
                return true;
            }
        }

        return false;
    }

    bool check(TokenType type)
    {

        if (isAtEnd())
            return false;

        return peek().type == type;
    }

    Token advance()
    {

        if (!isAtEnd())
        {
            current++;
        }

        return previous();
    }

    bool isAtEnd()
    {
        return peek().type ==
               TokenType::EOF_TOKEN;
    }

    Token peek()
    {
        return tokens[current];
    }

    Token previous()
    {
        return tokens[current - 1];
    }

    Token consume(
        TokenType type,
        string message)
    {

        if (check(type))
        {
            return advance();
        }

        throw runtime_error(message);
    }
};

// ================= TREE =================

void printTree(
    ASTNode *node,
    int indent,
    ofstream &out)
{

    string p(indent * 2, ' ');

    if (auto a =
            dynamic_cast<AssignNode *>(node))
    {

        out << p
            << "Assignment: "
            << a->name
            << "\n";

        printTree(a->expr, indent + 1, out);
    }

    else if (
        auto b =
            dynamic_cast<BinOpNode *>(node))
    {

        out << p
            << "Binary Operator: "
            << b->op
            << "\n";

        printTree(b->left, indent + 1, out);
        printTree(b->right, indent + 1, out);
    }

    else if (
        auto v =
            dynamic_cast<VarNode *>(node))
    {

        out << p
            << "Variable: "
            << v->name
            << "\n";
    }

    else if (
        auto n =
            dynamic_cast<NumberNode *>(node))
    {

        out << p
            << "Integer Value: "
            << n->value
            << "\n";
    }
}

// ================= MAIN =================

int main()
{

    ifstream in("input.bn");

    ofstream out("output.txt");

    try
    {

        if (!in.is_open())
        {
            throw runtime_error(
                "Cannot open input.bn");
        }

        stringstream buffer;

        buffer << in.rdbuf();

        string source = buffer.str();

        Lexer lexer(source);

        vector<Token> tokens =
            lexer.scanTokens();

        Parser parser(tokens);

        vector<ASTNode *> nodes =
            parser.parse();

        SymbolTable st;

        SemanticAnalyzer analyzer(st);

        analyzer.analyze(nodes);

        out << "=== TOKENS ===\n";

        for (auto t : tokens)
        {
            out << t.toString() << "\n";
        }

        out << "\n=== AST ===\n";

        for (auto node : nodes)
        {
            printTree(node, 0, out);
        }

        out << "\n=== SYMBOL TABLE ===\n";

        st.display(out);

        cout
            << "Compiler Finished Successfully\n";
    }

    catch (exception &e)
    {

        cerr
            << "Compiler Error: "
            << e.what()
            << "\n";

        out
            << "Compiler Error: "
            << e.what()
            << "\n";
    }

    return 0;
}