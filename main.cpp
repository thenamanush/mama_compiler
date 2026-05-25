#include <bits/stdc++.h>
using namespace std;

// ======================================================
// TOKEN TYPES(updated by tarek)
// ======================================================

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

// ======================================================
// AST NODES (updated- MHT)
// ======================================================

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

class StringNode : public ASTNode
{
public:
    string value;

    StringNode(string value)
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
    VarType type;
    string name;
    ASTNode *expr;

    AssignNode(
        VarType type,
        string name,
        ASTNode *expr)
    {
        this->type = type;
        this->name = name;
        this->expr = expr;
    }
};

class PrintNode : public ASTNode
{
public:
    ASTNode *expr;

    PrintNode(ASTNode *expr)
    {
        this->expr = expr;
    }
};

class IfNode : public ASTNode
{
public:
    ASTNode *condition;
    vector<ASTNode *> body;

    IfNode(
        ASTNode *condition,
        vector<ASTNode *> body)
    {
        this->condition = condition;
        this->body = body;
    }
};

// ======================================================
// SYMBOL TABLE (updated- MHT)
// ======================================================

struct Symbol
{
    VarType type;
    string value;
};

class SymbolTable
{
public:
    unordered_map<string, Symbol> table;

    void set(
        string name,
        VarType type,
        string value)
    {
        table[name] = {type, value};
    }

    bool contains(string name)
    {
        return table.find(name) != table.end();
    }

    Symbol get(string name)
    {
        return table[name];
    }
};

// ======================================================
// LEXER
// ======================================================

class Lexer
{
private:
    string source;
    vector<Token> tokens;

    int start = 0;
    int current = 0;
    int line = 1;

    unordered_map<string, TokenType> keywords = {
        {"মামাসংখ্যা", TokenType::MAMA_NUMBER},
        {"মামাঅক্ষর", TokenType::MAMA_STRING},
        {"যদি", TokenType::IF},
        {"দেখাও", TokenType::PRINT},
        {"নাহলে", TokenType::ELSE}};

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
            Token(
                TokenType::EOF_TOKEN,
                "",
                line));

        return tokens;
    }

private:
    bool isAtEnd()
    {
        return current >= source.size();
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

    void addToken(TokenType type)
    {
        tokens.push_back(
            Token(
                type,
                source.substr(start, current - start),
                line));
    }

    void scanToken()
    {
        char c = advance();

        switch (c)
        {
        case '(':
            addToken(TokenType::LPAREN);
            break;

        case ')':
            addToken(TokenType::RPAREN);
            break;

        case '{':
            addToken(TokenType::LBRACE);
            break;

        case '}':
            addToken(TokenType::RBRACE);
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

        case '>':
            addToken(TokenType::GREATER);
            break;

        case '<':
            addToken(TokenType::LESS);
            break;

        case ';':
            addToken(TokenType::SEMI);
            break;

        case '"':
            stringLiteral();
            break;

        case ' ':
        case '\r':
        case '\t':
            break;

        case '\n':
            line++;
            break;

        default:

            if (isdigit(c))
            {
                number();
            }

            else
            {
                identifier();
            }
        }
    }

    void stringLiteral()
    {
        while (peek() != '"' && !isAtEnd())
        {
            advance();
        }

        advance();

        addToken(TokenType::STRING_LITERAL);
    }

    void number()
    {
        while (isdigit(peek()))
        {
            advance();
        }

        addToken(TokenType::NUMBER);
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
};

// ======================================================
// PARSER
// ======================================================

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
        if (match({TokenType::MAMA_NUMBER,
                   TokenType::MAMA_STRING}))
        {
            return declaration();
        }

        if (match({TokenType::PRINT}))
        {
            return printStatement();
        }

        if (match({TokenType::IF}))
        {
            return ifStatement();
        }

        throw runtime_error("Invalid statement");
    }

    ASTNode *declaration()
    {
        VarType type;

        if (previous().type ==
            TokenType::MAMA_NUMBER)
        {
            type = VarType::INT;
        }
        else
        {
            type = VarType::STRING;
        }

        Token name =
            consume(TokenType::ID,
                    "Expected variable name");

        consume(TokenType::ASSIGN,
                "Expected '='");

        ASTNode *expr = expression();

        consume(TokenType::SEMI,
                "Expected ';'");

        return new AssignNode(
            type,
            name.lexeme,
            expr);
    }

    ASTNode *printStatement()
    {
        consume(TokenType::LPAREN,
                "Expected '('");

        ASTNode *expr = expression();

        consume(TokenType::RPAREN,
                "Expected ')'");

        consume(TokenType::SEMI,
                "Expected ';'");

        return new PrintNode(expr);
    }

    ASTNode *ifStatement()
    {
        consume(TokenType::LPAREN,
                "Expected '('");

        ASTNode *condition =
            expression();

        consume(TokenType::RPAREN,
                "Expected ')'");

        consume(TokenType::LBRACE,
                "Expected '{'");

        vector<ASTNode *> body;

        while (!check(TokenType::RBRACE))
        {
            body.push_back(statement());
        }

        consume(TokenType::RBRACE,
                "Expected '}'");

        return new IfNode(
            condition,
            body);
    }

    ASTNode *expression()
    {
        ASTNode *node = term();

        while (match({TokenType::PLUS,
                      TokenType::MINUS,
                      TokenType::GREATER,
                      TokenType::LESS}))
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

        while (match({TokenType::MUL,
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
                stoi(previous().lexeme));
        }

        if (match({TokenType::STRING_LITERAL}))
        {
            string val =
                previous().lexeme;

            val =
                val.substr(1,
                           val.size() - 2);

            return new StringNode(val);
        }

        if (match({TokenType::ID}))
        {
            return new VarNode(
                previous().lexeme);
        }

        if (match({TokenType::LPAREN}))
        {
            ASTNode *expr =
                expression();

            consume(TokenType::RPAREN,
                    "Expected ')'");

            return expr;
        }

        throw runtime_error(
            "Expected expression");
    }

    bool match(
        initializer_list<TokenType> types)
    {
        for (auto type : types)
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
            current++;

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

// ======================================================
// PYTHON GENERATOR
// ======================================================

class PythonGenerator
{
public:
    void generate(
        vector<ASTNode *> &nodes,
        ofstream &out)
    {
        for (auto node : nodes)
        {
            generateNode(node, out);
        }
    }

private:
    void generateNode(
        ASTNode *node,
        ofstream &out)
    {
        if (auto a =
                dynamic_cast<AssignNode *>(node))
        {
            out << a->name << " = ";

            generateExpr(a->expr, out);

            out << "\n";
        }

        else if (
            auto p =
                dynamic_cast<PrintNode *>(node))
        {
            out << "print(";

            generateExpr(p->expr, out);

            out << ")\n";
        }

        else if (
            auto i =
                dynamic_cast<IfNode *>(node))
        {
            out << "if ";

            generateExpr(
                i->condition,
                out);

            out << ":\n";

            for (auto stmt : i->body)
            {
                out << "    ";

                generateNode(stmt, out);
            }
        }
    }

    void generateExpr(
        ASTNode *node,
        ofstream &out)
    {
        if (auto n =
                dynamic_cast<NumberNode *>(node))
        {
            out << n->value;
        }

        else if (
            auto s =
                dynamic_cast<StringNode *>(node))
        {
            out << "\"" << s->value << "\"";
        }

        else if (
            auto v =
                dynamic_cast<VarNode *>(node))
        {
            out << v->name;
        }

        else if (
            auto b =
                dynamic_cast<BinOpNode *>(node))
        {
            generateExpr(b->left, out);

            out << " "
                << b->op
                << " ";

            generateExpr(b->right, out);
        }
    }
};

// ======================================================
// MAIN
// ======================================================

int main()
{
    ifstream in("input.bn");

    if (!in.is_open())
    {
        cout << "Cannot open input.bn\n";
        return 0;
    }

    stringstream buffer;

    buffer << in.rdbuf();

    string source = buffer.str();

    try
    {
        Lexer lexer(source);

        vector<Token> tokens =
            lexer.scanTokens();

        Parser parser(tokens);

        vector<ASTNode *> nodes =
            parser.parse();

        ofstream py("output.py");

        PythonGenerator generator;

        generator.generate(nodes, py);

        py.close();

        cout
            << "Compilation Successful\n";

        cout
            << "Generated output.py\n";
    }

    catch (exception &e)
    {
        cout
            << "Compiler Error: "
            << e.what()
            << "\n";
    }

    return 0;
}