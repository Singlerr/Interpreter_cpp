#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <wchar.h>
#include <wctype.h>
#include <vector>
#include <hash_map>
#include <map>
#include <queue>

using namespace std;
enum TokenType {
    IDENTIFIER = 0,
    ADD = 1,
    SUB = 2,
    MUL = 3,
    DIV = 4,
    MOD = 5,
    IF = 6,
    WHILE = 7,
    ELSE = 8,
    AND = 9,
    OR = 10,
    DOUBLE = 11,
    INTEGER = 12,
    LONG = 13,
    FLOAT = 14,
    UNKNOWN = 15,
    ERROR = 16,
    ASSIGN = 17,
    EQUAL = 18,
    START_QUOTE = 19,
    END_QUOTE = 20,
    END_IF = 21,
    END_WHILE = 22,
    START_BRACKET = 23,
    END_BRACKET = 24,
    STRING = 25
};

class Token {
public:
    wchar_t *value;
    enum TokenType type;

    Token(wchar_t *v, enum TokenType tokenType) {
        value = (wchar_t *) malloc(sizeof(v));
        wcscpy(value, v);
        type = tokenType;
    }
};

wchar_t *typetostr(enum TokenType tokenType);

bool isOperator(Token *token);

void compile(vector<Token> tokenStream);

vector<Token> lex(wchar_t line[]);

void clearBuffer(int *k, wchar_t *buffer, size_t size);

void appendBuffer(int *k, wchar_t *buffer, wchar_t c);

double findDoubleValue(wchar_t *);

wchar_t *findValue(wchar_t *);

bool valueExists(wchar_t *);

TokenType getType(wchar_t *);

TokenType classify(Token *token);

TokenType findType(wchar_t *value);

map<wchar_t *, wchar_t *> table;


int main(void) {
    setlocale(LC_ALL, "");
    wchar_t line[] = L"가나다가나다1233은 가나다+1235+156/129%130-130이다";
    vector<Token> stream = lex(line);
    compile(stream);
    return 0;
}

vector<Token> lex(wchar_t line[]) {
    //Token List
    vector<Token> tokenList;
    //The number of tokens
    int tNum = 0;
    int state = 0;
    //To add char to buffer, we need count.
    int k = 0;

    //The count of IF(만약). It means how many IF appeared to read appreciate number of ENDIF
    int ifCount = 0;
    //The count of WHILE(동안). It means how many WHILE appeared to read appreciate number of ENDWHILE
    int whileCount = 0;
    wchar_t *buffer = (wchar_t *) malloc(sizeof(line) * 100);
    wmemset(buffer, 0, sizeof(buffer));


    for (int i = 0; i < wcslen(line); i++) {
        wchar_t c = line[i];
        wchar_t c2 = i + 1 < wcslen(line) ? line[i + 1] : EOF;
        switch (c) {
            case L'은': {
                //An identifier appeared before and 은 appears. Then save the identifier and clear buffer to write new characters.
                if (state == 1) {
                    state++;//Now state is 2
                    //Save the token
                    Token token(buffer, UNKNOWN);
                    tokenList.push_back(token);
                    //And add assign token to notify that this is in assign statement.
                    Token assignToken(L"은", ASSIGN);
                    tokenList.push_back(assignToken);
                    //Clear buffer
                    wmemset(buffer, 0, sizeof(buffer));
                    k = 0;
                } else {
                    //We don't know what it is. Maybe identifier?
                    //Anyway we have to record this.
                    buffer[k++] = c;
                }
                break;
            }
            case L'는': {
                //An identifier appeared before and 은 appears.
                if (state == 1) {
                    state++;//Now state is 2
                } else {
                    //We don't know what it is. Maybe identifier?
                    //Anyway we have to record this.
                    //appendBuffer(&k, buffer, c);
                    buffer[k++] = c;
                }
                break;
            }
                //There are two ways. One is just the word '이' another is '이고' which means AND.
            case L'이': {
                //When '이고' appeared
                if (c2 != EOF && c2 == L'고') {
                    //Then save former value.
                    Token token(buffer, UNKNOWN);
                    tokenList.push_back(token);
                    //And clear the buffer.
                    wmemset(buffer, 0, sizeof(buffer));
                    k = 0;
                    break;
                }
                //A value appeared and it's time to end the assign statement.
                if (state == 2) {
                    state++;
                } else {
                    //We don't know what it is. Maybe identifier?
                    //Anyway we have to record this.
                    buffer[k++] = c;
                }
                break;
            }
            case L'다': {
                //We checked 이 appeared before and now 다 should appeared
                if (state == 3) {
                    //That's it! Now we have to create token to save the value.
                    Token token(buffer, UNKNOWN);
                    tokenList.push_back(token);
                    //We should tokenize this.
                    //And clear the buffer so that another token can use it.
                    wmemset(buffer, 0, sizeof(buffer));
                    k = 0;
                    //Finally, reset the state
                    state = 0;
                } else {
                    //We don't know what it is. Maybe identifier?
                    //Anyway we have to record this.
                    buffer[k++] = c;
                }
                break;
            }
            case L'+': {
                //+ appeared after something(it can be an identifier or number)
                if (state > 0) {
                    //+ appeared after something, so we have to create a token about it. ex) abc + -> we must save 'abc'.
                    //We save the token as UNKNOWN type because we dont know whether this is identifier or number.
                    Token unknown(buffer, UNKNOWN);
                    tokenList.push_back(unknown);
                    //And clear the buffer
                    wmemset(buffer, 0, sizeof(buffer));
                    k = 0;
                    //Reset the state to make it enable to recognize other things for next character.
                    //First, if state is not 2(which means that it is in assign statement)
                    if (state != 2)
                        state = 0;
                    //And also save ADD token.
                    Token add(L"+", ADD);
                    tokenList.push_back(add);
                }
                break;
            }
            case L'-': {
                //- appeared after something(it can be an identifier or number)
                if (state > 0) {

                    //- appeared after something, so we have to create a token about it. ex) abc - -> we must save 'abc'.
                    //We save the token as UNKNOWN type because we dont know whether this is identifier or number.
                    Token unknown(buffer, UNKNOWN);
                    tokenList.push_back(unknown);
                    //And clear the buffer
                    wmemset(buffer, 0, sizeof(buffer));
                    k = 0;
                    //Reset the state to make it enable to recognize other things for next character.
                    //First, if state is not 2(which means that it is in assign statement)
                    if (state != 2)
                        state = 0;
                    //And also save SUB token.
                    Token sub(L"-", SUB);
                    tokenList.push_back(sub);
                }
                break;
            }
            case L'/': {
                // / appeared after something(it can be an identifier or number)
                if (state > 0) {

                    // / appeared after something, so we have to create a token about it. ex) abc / -> we must save 'abc'.
                    //We save the token as UNKNOWN type because we dont know whether this is identifier or number.
                    Token unknown(buffer, UNKNOWN);
                    tokenList.push_back(unknown);
                    //And clear the buffer
                    wmemset(buffer, 0, sizeof(buffer));
                    k = 0;
                    //Reset the state to make it enable to recognize other things for next character.
                    //First, if state is not 2(which means that it is in assign statement)
                    if (state != 2)
                        state = 0;
                    //And also save ADD token.
                    Token div(L"/", DIV);
                    tokenList.push_back(div);
                }
                break;
            }
            case L'%': {
                // % appeared after something(it can be an identifier or number)
                if (state > 0) {

                    // % appeared after something, so we have to create a token about it. ex) abc % -> we must save 'abc'.
                    //We save the token as UNKNOWN type because we dont know whether this is identifier or number.
                    Token unknown(buffer, UNKNOWN);
                    tokenList.push_back(unknown);
                    //And clear the buffer
                    wmemset(buffer, 0, sizeof(buffer));
                    k = 0;
                    //Reset the state to make it enable to recognize other things for next character.
                    //First, if state is not 2(which means that it is in assign statement)
                    if (state != 2)
                        state = 0;
                    //And also save MOD token.
                    Token mod(L"/", MOD);
                    tokenList.push_back(mod);
                }
                break;
            }
            case L'=': {
                //When EQUAL sign appeared
                if (c2 != EOF && c2 == L'=') {
                    //It means that there is something before == appeared.
                    if (state > 0) {
                        // == appeared after something, so we have to create a token about it. ex) abc == -> we must save 'abc'.
                        //We save the token as UNKNOWN type because we dont know whether this is identifier or number.
                        Token unknown(buffer, UNKNOWN);
                        tokenList.push_back(unknown);
                        //And clear the buffer
                        wmemset(buffer, 0, sizeof(buffer));
                        k = 0;
                        //Reset the state to make it enable to recognize other things for next character.
                        //First, if state is not 2(which means that it is in assign statement)
                        if (state != 2)
                            state = 0;
                        //And also save EQUAL token.
                        Token equal(L"==", EQUAL);
                        tokenList.push_back(equal);
                        //Finally, because it used two characters, so we skip 1 characters
                        i += 1;
                    }
                }
                break;
            }
            case L'&': {
                //When AND sign appeared
                if (c2 != EOF && c2 == L'&') {
                    //It means that there is something before == appeared.
                    if (state > 0) {
                        // && appeared after something, so we have to create a token about it. ex) abc && -> we must save 'abc'.
                        //We save the token as UNKNOWN type because we dont know whether this is identifier or number.
                        Token unknown(buffer, UNKNOWN);
                        tokenList.push_back(unknown);
                        //And clear the buffer
                        wmemset(buffer, 0, sizeof(buffer));
                        k = 0;
                        //Reset the state to make it enable to recognize other things for next character.
                        //First, if state is not 2(which means that it is in assign statement)
                        if (state != 2)
                            state = 0;
                        //And also save AND token.
                        Token and_(L"&&", AND);
                        tokenList.push_back(and_);
                        //Finally, because it used two characters, so we skip 1 characters
                        i += 1;
                    }
                }
                break;
            }
            case L'|': {
                //When OR sign appeared
                if (c2 != EOF && c2 == L'|') {
                    //It means that there is something before || appeared.
                    if (state > 0) {
                        // OR appeared after something, so we have to create a token about it. ex) abc || -> we must save 'abc'.
                        //We save the token as UNKNOWN type because we dont know whether this is identifier or number.
                        Token unknown(buffer, UNKNOWN);
                        tokenList.push_back(unknown);
                        //And clear the buffer
                        wmemset(buffer, 0, sizeof(buffer));
                        k = 0;
                        //Reset the state to make it enable to recognize other things for next character.
                        //First, if state is not 2(which means that it is in assign statement)
                        if (state != 2)
                            state = 0;
                        //And also save OR token.
                        Token or_(L"||", OR);
                        tokenList.push_back(or_);
                        //Finally, because it used two characters, so we skip 1 characters
                        i += 1;
                    }
                }
                break;
            }
                //There are two ways. One is just the word '또' another is '또는' which means OR.
            case L'또': {
                //OR sign('또는') appeared
                if (c2 != EOF && c2 == L'는') {
                    //It means that there is something before || appeared.
                    if (state > 0) {
                        // / appeared after something, so we have to create a token about it. ex) abc || -> we must save 'abc'.
                        //We save the token as UNKNOWN type because we dont know whether this is identifier or number.
                        Token unknown(buffer, UNKNOWN);
                        tokenList.push_back(unknown);
                        //And clear the buffer
                        wmemset(buffer, 0, sizeof(buffer));
                        k = 0;
                        //Reset the state to make it enable to recognize other things for next character.
                        //First, if state is not 2(which means that it is in assign statement)
                        if (state != 2)
                            state = 0;
                        //And also save OR token.
                        Token or_(L"또는", OR);
                        tokenList.push_back(or_);
                        //Finally, because it used two characters, so we skip 1 characters
                        i += 1;
                    }
                } else {
                    //Hey, it is just '또', maybe identifier? Anyway, record it.
                    buffer[k++] = c;
                }
                break;
            }
                //When string letter appeared
            case L'"': {
                //It means that first " appeared, which means that it is end quote mark!
                if (state == 0) {
                    // " appeared after something, so we have to create a token about it. ex) "abc -> we must save 'abc'.
                    //We save the token as UNKNOWN type because we dont know whether this is identifier or number.
                    Token unknown(buffer, UNKNOWN);
                    tokenList.push_back(unknown);
                    //And clear the buffer
                    wmemset(buffer, 0, sizeof(buffer));
                    k = 0;
                    //In quote, no words are effected by keywords.
                    state = 7;
                    //And also save QUOTE token.
                    //Token quote(L"\"", START_QUOTE);
                    //tokenList.push_back(quote);
                    //It means that last " appeared, which means that it is start quote mark!
                } else {
                    // " appeared after something, so we have to create a token about it. ex) abc " -> we must save 'abc'.
                    //We save the token as STRING type. Because any characters between " are read as String.
                    Token unknown(buffer, STRING);
                    tokenList.push_back(unknown);
                    //And clear the buffer
                    wmemset(buffer, 0, sizeof(buffer));
                    k = 0;
                    //Reset the state to make it enable to recognize other things for next character.
                    //First, if state is not 2(which means that it is in assign statement)
                    if (state != 2)
                        state = 0;
                    //And also save QUOTE token.
                    //Token quote(L"\"", END_QUOTE);
                    //tokenList.push_back(quote);

                }
                break;
            }
            case L'만': {
                //When '만약' appeared (it means IF)
                if (c2 != EOF && c2 == L'약') {
                    //만약 must appear at the first of the statement.
                    if (state == 0) {
                        //Save IF token
                        Token if_(L"만약", IF);
                        tokenList.push_back(if_);
                        //Set the state to read condition statement.
                        state = 5;
                        //Finally, because it used two characters, so we skip 1 characters
                        i += 1;
                    }
                } else {
                    //Hey, it is just '만', maybe identifier? Anyway, record it.
                    buffer[k++] = c;
                }
                break;
            }
            case L'동': {
                //When '동안' appeared (it means WHILE)
                if (c2 != EOF && c2 == L'안') {
                    //동안 must appear at the first of the statement.
                    if (state == 0) {
                        state = 6;
                        //Finally, because it used two characters, so we skip 1 characters
                        i += 1;
                    }
                } else {
                    //Hey, it is just '동', maybe identifier? Anyway, record it.
                    buffer[k++] = c;
                }
                break;
            }
                //This requires only if 만약(IF), 동안(WHILE) appeared before.
            case L',': {
                //This means that , appeared after IF because state == 5 after 만약 appeared.
                if (state == 5) {
                    //TODO("Add another statement recognizer")
                    //Maybe set the state to 0 to read any statements?
                    state = 0;
                    break;
                }
                //This means that , appeared after WHILE because state == 6 after 동안 appeared.
                if (state == 6) {
                    //TODO("Add another statement recognizer")
                    //Maybe set the state to 0 to read any statements?
                    state = 0;
                    break;
                }

            }
                //It applies any states.
                //But there are some situations that this bracket must not be applied. This works by if condition.
            case L'(': {
                //Anyway, we have to tokenize.
                Token token(L"(", START_BRACKET);
                tokenList.push_back(token);

                //Then clear the buffer.
                wmemset(buffer, 0, sizeof(buffer));
                k = 0;
                break;
            }
            case L')': {
                //We have to check if there is START_BRACKET before. It should be treated when investigate tokens.
                //Anyway, we have to tokenize.
                Token token(L")", END_BRACKET);
                tokenList.push_back(token);

                //Then clear the buffer.
                wmemset(buffer, 0, sizeof(buffer));
                k = 0;
                break;
            }
            default: {
                if (c == L' ')
                    continue;
                //Expecting 은/는 to appear after and now we should record the name of an identifier.
                if (state == 0) {
                    state++;
                }
                //A state of reading an identifier
                if (state == 1) {
                    //Recording the name of an identifier.
                    buffer[k++] = c;
                }

                //A state of reading value before 이다
                if (state == 2) {
                    //A buffer should record value until 이다 appears.
                    buffer[k++] = c;
                }
                //A state of reading IF condition statement.
                if (state == 5) {
                    buffer[k++] = c;
                }
                //A state of reading WHILE condition statement.
                if (state == 6) {
                    buffer[k++] = c;
                }
                //A state of reading string between "
                if (state == 7) {
                    buffer[k++] = c;
                }
                break;
            }
        }
    }
    for (int i = 0; i < tokenList.size(); i++) {
        Token *token = &tokenList.at(i);
        enum TokenType tokenType = classify(token);
        token->type = tokenType;
        if (tokenType == IDENTIFIER) {
            //TODO("In compiling stage, setting the value of token")
            table.insert(pair<wchar_t *, wchar_t *>(token->value, L""));
        }
    }
    return tokenList;
}


enum TokenType classify(Token *token) {
    //We have to classify token type only if its type is UNKNOWN.
    enum TokenType type = token->type;
    wchar_t *value = token->value;
    if (type != UNKNOWN)
        return type;
    int state = 0;
    //Let's go through all characters!
    for (int i = 0; i < wcslen(value); i++) {
        wchar_t c = value[i];
        //If a character is literal or _, not a number.
        if ((c >= L'가' && c <= L'힣') || (c >= L'a' && c <= L'z') || (c >= L'A' && c <= L'Z') || (c == L'_')) {
            //Check state and increase the state if needs.
            //It is repeating part in DFA.
            if (state == 0)
                state++;

            //This must be treated as error.
            //A character after a number is not valid, even if it is an identifier.
            if (state > 1 && state < 4) {
                //ERROR
                token->type = ERROR;
                return ERROR;
            }
        }
    }
    //Check if a whole value is double or integer.
    wchar_t *temp;
    if (wcstod((const wchar_t *) value, &temp) != 0) {
        if (state == 0)
            state = 2;
        //If a character is integer
    } else {
        if (state == 0) {
            bool flag = false;
            for (int i = 0; i < wcslen(value); i++)
                if (iswalpha(value[i]))
                    flag = true;

            if (!flag)
                state = 3;
        }
    }
    if (state == 2) {
        token->type = DOUBLE;
        return DOUBLE;
    } else if (state == 3) {
        token->type = INTEGER;
        return INTEGER;
    } else if (state == 1) {
        token->type = IDENTIFIER;
        return IDENTIFIER;
    }
    return UNKNOWN;
}

void compile(vector<Token> tokenStream) {
    /**
     * ATTENTION! 별개로 container에 등록될 때는 ADD일 때만 성립한다!
     * 그러니까, ADD일 경우 그리고 뒤에 MUL,DIV와 같이 우선순위가 높은 연산자가 없을 때,
     * 그때는 left, right 토큰 값을 더해서 valueContainer에 저장하는 게 아니라
     * left 값을 valueContainer에 더하고, 다음 루프로 넘어간다음,
     * MUL은... 아직 못정함. 어떻게 해야할까? TokenType을 MUL로 해서 등록한 다음,
     * 나중에 값을 처리할 때 MUL이면 그 위치의 이전 값과 곱하는 것으로 해야할까?
     * -> 해결법: DOUBLE1 ADD DOUBLE2 일 경우, DOUBLE1 왼쪽의 토큰을 확인해서 + 일 경우
     * valueContainer/bracketContainer에 추가, DOUBLE2 오른쪽에 *,/ 가 있을 경우는 그냥 스킵(현재 상태에서)
     * 스킵하면 계속 앞으로 이동하면서 *,/이 있는 지점으로 갈테고 거기서 또 처리하면 됨
     * *./ 일 경우에는 (DOUBLE1 * DOUBLE2) DOUBLE 1왼쪽에 있는 토큰 확인해서  + 일 경우 곱한 결과 추가
     * +) SUB은 + (- 값)으로 처리!
     */
    double double_value = 0;
    long long_value = 0;
    wstring buffer;
    for (int i = 0; i < tokenStream.size(); i++) {
        Token *currentToken = &tokenStream.at(i);
        Token *nextToken = i + 1 < tokenStream.size() ? &tokenStream.at(i + 1) : NULL;
        //If this tokenStream implies ASSIGN statement.
        if (currentToken->type == IDENTIFIER && nextToken != NULL && nextToken->type == ASSIGN) {
            double leftValue = 0;
            double rightValue = 0;
            //It saves data per state. If state is 1, then data saved in position 0,which means that if statement is in 1 bracket, its parsed data will be inserted in position 1.
            //When got out of brackets, then sum of bracketContainer returns.
            vector<pair<TokenType, wchar_t *> > bracketContainer;
            //The value container of one statement. Not bracket container. It is just used because to save value with types.
            vector<pair<TokenType, wchar_t *> > valueContainer;
            //The reading state, specifically string.
            //If strState > 0, then it means that there are strings after and we should concat number,not add
            //Deprecated
            int strState = 0;
            //The reading state.
            //state -> How many START_BRACKET appeared.
            //When encountered START_BRACKET, then state++ to record number of brackets
            //When encountered END_BRACKET, then state--, if state < 0 then there are syntax errors.
            int state = 0;
            for (int j = i + 2; j < tokenStream.size(); j++) {
                Token *subToken = &tokenStream.at(j);
                Token *left = j - 1 >= 0 ? &tokenStream.at(j - 1) : NULL;
                Token *right = j + 1 < tokenStream.size() ? &tokenStream.at(j + 1) : NULL;
                printf("C: %ls, N: %ls\n", typetostr(subToken->type), typetostr(right->type));
                switch (subToken->type) {
                    case ADD: {
                        if (left == NULL || right == NULL) {
                            perror("An operator must have both side");
                            break;
                        }
                        if (right->type != STRING && (right->type == IDENTIFIER && findType(right->value) != STRING)) {
                            Token *nextToken = j + 2 < tokenStream.size() ? &tokenStream.at(j + 2) : NULL;
                            //If next token exists and there is MUL or DIV operator,which must be operated first.
                            if (nextToken != NULL &&
                                (nextToken->type == MUL || nextToken->type == DIV || nextToken->type == MOD)) {
                                //If next token is an identifier.
                                if (left->type == IDENTIFIER && valueExists(left->value) &&
                                    getType(left->value) != STRING) {
                                    //Then save current token value and move to MUL or DIV location.
                                    //If in brackets.
                                    if (state > 0) {
                                        //Then save to bracket container.
                                        //If the state already exists.
                                        if (state < tokenStream.size()) {
                                            if (bracketContainer.at(state).first == STRING) {
                                                wchar_t *value = wcscat(bracketContainer.at(state).second,
                                                                        findValue(left->value));
                                                bracketContainer.insert(bracketContainer.begin() + state,
                                                                        pair<TokenType, wchar_t *>(STRING, value));
                                            } else {
                                                double v = wcstod(bracketContainer.at(state).second, NULL) +
                                                           findDoubleValue(left->value);
                                                wchar_t *buffer = (wchar_t *) malloc(sizeof(left->value) * 10);
                                                swprintf(buffer, sizeof(left->value) * 10, L"%f", v);
                                                bracketContainer.insert(bracketContainer.begin() + state,
                                                                        pair<TokenType, wchar_t *>(DOUBLE, buffer));
                                            }
                                        } else {
                                            bracketContainer.insert(bracketContainer.begin() + state,
                                                                    pair<TokenType, wchar_t *>(DOUBLE,
                                                                                               findValue(left->value)));
                                        }
                                    } else {
                                        //Then save to value container.
                                        valueContainer.push_back(
                                                pair<TokenType, wchar_t *>(DOUBLE, findValue(left->value)));
                                    }

                                }
                                //If next token is an identifier and not string
                                if (left->type == IDENTIFIER) {
                                    //If value does not exists.
                                    if (!valueExists(left->value)) {
                                        //Throw error
                                        throw invalid_argument("Expected variable but not found");
                                    }
                                    //Then save current token value and move to MUL or DIV location.
                                    //If in brackets.
                                    if (state > 0) {
                                        //Then save to bracket container.
                                        //If the state already exists.
                                        if (state < tokenStream.size()) {
                                            if (bracketContainer.at(state).first == STRING) {
                                                wchar_t *value = wcscat(bracketContainer.at(state).second,
                                                                        findValue(left->value));
                                                bracketContainer.insert(bracketContainer.begin() + state,
                                                                        pair<TokenType, wchar_t *>(STRING, value));
                                            } else {
                                                if (findType(left->value) == STRING) {
                                                    bracketContainer.insert(bracketContainer.begin() + state,
                                                                            pair<TokenType, wchar_t *>(STRING,
                                                                                                       wcscat(bracketContainer.at(
                                                                                                               state).second,
                                                                                                              findValue(
                                                                                                                      left->value))));
                                                } else {
                                                    double v = wcstod(bracketContainer.at(state).second, NULL) +
                                                               findDoubleValue(left->value);
                                                    wchar_t *buffer = (wchar_t *) malloc(sizeof(left->value) * 10);
                                                    swprintf(buffer, sizeof(left->value) * 10, L"%f", v);
                                                    bracketContainer.insert(bracketContainer.begin() + state,
                                                                            pair<TokenType, wchar_t *>(DOUBLE, buffer));
                                                }

                                            }

                                        } else {
                                            if (findType(left->value) == STRING) {
                                                bracketContainer.insert(bracketContainer.begin() + state,
                                                                        pair<TokenType, wchar_t *>(STRING, findValue(
                                                                                left->value)));
                                            } else {
                                                bracketContainer.insert(bracketContainer.begin() + state,
                                                                        pair<TokenType, wchar_t *>(DOUBLE, findValue(
                                                                                left->value)));
                                            }

                                        }
                                    } else {
                                        //Then save to value container.
                                        valueContainer.push_back(pair<TokenType, wchar_t *>(DOUBLE, left->value));
                                    }

                                } else {
                                    //Then save current token value and move to MUL or DIV location.
                                    //If in brackets.
                                    if (state > 0) {
                                        //Then save to bracket container.
                                        //If the state already exists.
                                        if (state < tokenStream.size()) {
                                            if (bracketContainer.at(state).first == STRING) {
                                                wchar_t *value = wcscat(bracketContainer.at(state).second, left->value);
                                                bracketContainer.insert(bracketContainer.begin() + state,
                                                                        pair<TokenType, wchar_t *>(STRING, value));
                                            } else {
                                                if (left->type == STRING) {
                                                    bracketContainer.insert(bracketContainer.begin() + state,
                                                                            pair<TokenType, wchar_t *>(STRING,
                                                                                                       wcscat(bracketContainer.at(
                                                                                                               state).second,
                                                                                                              left->value)));
                                                } else {
                                                    double v = wcstod(bracketContainer.at(state).second, NULL) +
                                                               wcstod(left->value, NULL);
                                                    wchar_t *buffer = (wchar_t *) malloc(sizeof(left->value) * 10);
                                                    swprintf(buffer, sizeof(left->value) * 10, L"%f", v);
                                                    bracketContainer.insert(bracketContainer.begin() + state,
                                                                            pair<TokenType, wchar_t *>(DOUBLE, buffer));
                                                }

                                            }

                                        } else {
                                            if (left->type == STRING) {
                                                bracketContainer.insert(bracketContainer.begin() + state,
                                                                        pair<TokenType, wchar_t *>(STRING,
                                                                                                   left->value));
                                            } else {
                                                double v = wcstod(left->value, NULL);
                                                wchar_t *buffer = (wchar_t *) malloc(sizeof(left->value) * 10);
                                                swprintf(buffer, sizeof(left->value) * 10, L"%f", v);
                                                bracketContainer.insert(bracketContainer.begin() + state,
                                                                        pair<TokenType, wchar_t *>(DOUBLE, buffer));
                                            }
                                        }
                                    } else {
                                        //Then save to value container.
                                        if (left->type == STRING) {
                                            valueContainer.push_back(pair<TokenType, wchar_t *>(STRING, left->value));
                                        } else {
                                            valueContainer.push_back(pair<TokenType, wchar_t *>(DOUBLE, left->value));
                                        }

                                    }
                                }
                                //Move to MUL or DIV location.
                                j += 2;
                                continue;
                                //If there is no more MUL or DIV or MOD.
                            } else if (nextToken != NULL) {
                                //Then just save to container.
                                if (left->type == IDENTIFIER) {
                                    //It is an identifier but not exists then throw error.
                                    if (!valueExists(left->value)) {
                                        throw invalid_argument("Expected variable but not found");
                                    }
                                    //If in brackets.
                                    if (state > 0) {
                                        //Then save to bracket container.
                                        //If there is value already.
                                        if (state < tokenStream.size()) {
                                            //If precedent value is string
                                            if (bracketContainer.at(state).first == STRING) {
                                                //Then concat and save
                                                bracketContainer.insert(bracketContainer.begin() + state,
                                                                        pair<TokenType, wchar_t *>(STRING,
                                                                                                   wcscat(bracketContainer.at(
                                                                                                           state).second,
                                                                                                          findValue(
                                                                                                                  left->value))));
                                            } else {
                                                //If current token type is string
                                                if (findType(left->value) == STRING) {
                                                    //Then concat and save
                                                    bracketContainer.insert(bracketContainer.begin() + state,
                                                                            pair<TokenType, wchar_t *>(STRING,
                                                                                                       wcscat(bracketContainer.at(
                                                                                                               state).second,
                                                                                                              findValue(
                                                                                                                      left->value))));
                                                } else {
                                                    //Convert bracket container value and current value to double and add them.
                                                    double v = wcstod(bracketContainer.at(state).second, NULL) +
                                                               findDoubleValue(left->value);
                                                    wchar_t *buffer = (wchar_t *) malloc(sizeof(left->value) * 10);
                                                    swprintf(buffer, sizeof(left->value) * 10, L"%f", v);
                                                    bracketContainer.insert(bracketContainer.begin() + state,
                                                                            pair<TokenType, wchar_t *>(DOUBLE, buffer));
                                                }
                                            }

                                        } else {
                                            //If current token type is string
                                            if (findType(left->value) == STRING) {
                                                //Then save as string
                                                bracketContainer.insert(bracketContainer.begin() + state,
                                                                        pair<TokenType, wchar_t *>(STRING,
                                                                                                   findValue(
                                                                                                           left->value)));
                                            } else {
                                                bracketContainer.insert(bracketContainer.begin() + state,
                                                                        pair<TokenType, wchar_t *>(DOUBLE, findValue(
                                                                                left->value)));
                                            }
                                        }
                                    } else {
                                        //It is not in bracket. Then just add to valueContainer.
                                        //If current token type is string
                                        if (findType(left->value) == STRING) {
                                            //Then save as string
                                            valueContainer.insert(valueContainer.begin() + state,
                                                                  pair<TokenType, wchar_t *>(STRING,
                                                                                             findValue(left->value)));
                                        } else {
                                            //Then save as double
                                            valueContainer.insert(valueContainer.begin() + state,
                                                                  pair<TokenType, wchar_t *>(DOUBLE,
                                                                                             findValue(left->value)));
                                        }
                                    }
                                    //It is not an identifier.
                                } else {
                                    //Then save current token value and move to MUL or DIV location.
                                    //If in brackets.
                                    if (state > 0) {
                                        //Then save to bracket container.
                                        //If the state already exists.
                                        if (state < tokenStream.size()) {
                                            if (bracketContainer.at(state).first == STRING) {
                                                wchar_t *value = wcscat(bracketContainer.at(state).second, left->value);
                                                bracketContainer.insert(bracketContainer.begin() + state,
                                                                        pair<TokenType, wchar_t *>(STRING, value));
                                            } else {
                                                if (left->type == STRING) {
                                                    bracketContainer.insert(bracketContainer.begin() + state,
                                                                            pair<TokenType, wchar_t *>(STRING,
                                                                                                       wcscat(bracketContainer.at(
                                                                                                               state).second,
                                                                                                              left->value)));
                                                } else {
                                                    double v = wcstod(bracketContainer.at(state).second, NULL) +
                                                               wcstod(left->value, NULL);
                                                    wchar_t *buffer = (wchar_t *) malloc(sizeof(left->value) * 10);
                                                    swprintf(buffer, sizeof(left->value) * 10, L"%f", v);
                                                    bracketContainer.insert(bracketContainer.begin() + state,
                                                                            pair<TokenType, wchar_t *>(DOUBLE, buffer));
                                                }

                                            }

                                        } else {
                                            if (left->type == STRING) {
                                                bracketContainer.insert(bracketContainer.begin() + state,
                                                                        pair<TokenType, wchar_t *>(STRING,
                                                                                                   left->value));
                                            } else {
                                                double v = wcstod(left->value, NULL);
                                                wchar_t *buffer = (wchar_t *) malloc(sizeof(left->value) * 10);
                                                swprintf(buffer, sizeof(left->value) * 10, L"%f", v);
                                                bracketContainer.insert(bracketContainer.begin() + state,
                                                                        pair<TokenType, wchar_t *>(DOUBLE, buffer));
                                            }
                                        }
                                    } else {
                                        //Then save to value container.
                                        if (left->type == STRING) {
                                            valueContainer.push_back(pair<TokenType, wchar_t *>(STRING, left->value));
                                        } else {
                                            valueContainer.push_back(pair<TokenType, wchar_t *>(DOUBLE, left->value));
                                        }

                                    }
                                }
                            }
                            //Left token type is string and right token type is not string
                        } else if (left->type == STRING) {

                            //If in brackets
                            if (state > 0) {
                                //If the state already exists
                                if (state < bracketContainer.size()) {
                                    bracketContainer.insert(bracketContainer.begin() + state,
                                                            pair<TokenType, wchar_t *>(STRING,
                                                                                       wcscat(bracketContainer.at(
                                                                                               state).second,
                                                                                              left->value)));
                                } else {
                                    bracketContainer.insert(bracketContainer.begin() + state,
                                                            pair<TokenType, wchar_t *>(STRING, left->value));
                                }
                            } else {
                                valueContainer.push_back(pair<TokenType, wchar_t *>(STRING, left->value));
                            }
                            //Left token type is not string and right token type is not string
                        } else {
                            //If type of left token is an identifier
                            if (left->type == IDENTIFIER) {
                                //If the identifier is not registered
                                if (!valueExists(left->value)) {
                                    //Throw error
                                    throw invalid_argument("Expected variable but not found");
                                }
                                //If type of identifier is string
                                if (findType(left->value) == STRING) {
                                    //If in brackets
                                    if (state > 0) {
                                        //If the state is already in list.
                                        if (state < bracketContainer.size()) {
                                            bracketContainer.insert(bracketContainer.begin() + state,
                                                                    pair<TokenType, wchar_t *>(STRING,
                                                                                               wcscat(bracketContainer.at(
                                                                                                       state).second,
                                                                                                      findValue(
                                                                                                              left->value))));
                                            //If not.
                                        } else {
                                            bracketContainer.insert(bracketContainer.begin() + state,
                                                                    pair<TokenType, wchar_t *>(STRING,
                                                                                               findValue(left->value)));
                                        }
                                    } else {
                                        valueContainer.push_back(
                                                pair<TokenType, wchar_t *>(STRING, findValue(left->value)));
                                    }
                                } else {
                                    //If in brackets
                                    if (state > 0) {
                                        //If the state is already in list.
                                        if (state < bracketContainer.size()) {
                                            double v = wcstod(bracketContainer.at(state).second, NULL) +
                                                       findDoubleValue(left->value);
                                            wchar_t *buffer = (wchar_t *) malloc(sizeof(findValue(left->value)) * 10);
                                            swprintf(buffer, sizeof(left->value) * 10, L"%f", v);
                                            bracketContainer.insert(bracketContainer.begin() + state,
                                                                    pair<TokenType, wchar_t *>(DOUBLE, buffer));
                                            //If not.
                                        } else {
                                            bracketContainer.insert(bracketContainer.begin() + state,
                                                                    pair<TokenType, wchar_t *>(DOUBLE,
                                                                                               findValue(left->value)));
                                        }
                                    } else {
                                        valueContainer.push_back(
                                                pair<TokenType, wchar_t *>(DOUBLE, findValue(left->value)));
                                    }
                                }
                            } else {
                                //If type of left token is an identifier
                                if (left->type == IDENTIFIER) {
                                    //If the identifier is not registered
                                    if (!valueExists(left->value)) {
                                        //Throw error
                                        throw invalid_argument("Expected variable but not found");
                                    }
                                    //If type of identifier is string
                                    if (findType(left->value) == STRING) {
                                        //If in brackets
                                        if (state > 0) {
                                            //If the state is already in list.
                                            if (state < bracketContainer.size()) {
                                                bracketContainer.insert(bracketContainer.begin() + state,
                                                                        pair<TokenType, wchar_t *>(STRING,
                                                                                                   wcscat(bracketContainer.at(
                                                                                                           state).second,
                                                                                                          findValue(
                                                                                                                  left->value))));
                                                //If not.
                                            } else {
                                                bracketContainer.insert(bracketContainer.begin() + state,
                                                                        pair<TokenType, wchar_t *>(STRING,
                                                                                                   findValue(
                                                                                                           left->value)));
                                            }
                                        } else {
                                            valueContainer.push_back(
                                                    pair<TokenType, wchar_t *>(STRING, findValue(left->value)));
                                        }
                                    } else {
                                        //If in brackets
                                        if (state > 0) {
                                            //If the state is already in list.
                                            if (state < bracketContainer.size()) {
                                                double v = wcstod(bracketContainer.at(state).second, NULL) +
                                                           findDoubleValue(left->value);
                                                wchar_t *buffer = (wchar_t *) malloc(
                                                        sizeof(findValue(left->value)) * 10);
                                                swprintf(buffer, sizeof(findValue(left->value)) * 10, L"%f", v);
                                                bracketContainer.insert(bracketContainer.begin() + state,
                                                                        pair<TokenType, wchar_t *>(DOUBLE, buffer));
                                                //If not.
                                            } else {
                                                bracketContainer.insert(bracketContainer.begin() + state,
                                                                        pair<TokenType, wchar_t *>(DOUBLE,
                                                                                                   findValue(
                                                                                                           left->value)));
                                            }
                                        } else {
                                            valueContainer.push_back(
                                                    pair<TokenType, wchar_t *>(DOUBLE, findValue(left->value)));
                                        }
                                    }
                                } else {
                                    //If type of identifier is string
                                    if (left->type == STRING) {
                                        //If in brackets
                                        if (state > 0) {
                                            //If the state is already in list.
                                            if (state < bracketContainer.size()) {
                                                bracketContainer.insert(bracketContainer.begin() + state,
                                                                        pair<TokenType, wchar_t *>(STRING,
                                                                                                   wcscat(bracketContainer.at(
                                                                                                           state).second,
                                                                                                          left->value)));
                                                //If not.
                                            } else {
                                                bracketContainer.insert(bracketContainer.begin() + state,
                                                                        pair<TokenType, wchar_t *>(STRING,
                                                                                                   left->value));
                                            }
                                        } else {
                                            valueContainer.push_back(
                                                    pair<TokenType, wchar_t *>(STRING, left->value));
                                        }
                                    } else {
                                        //If in brackets
                                        if (state > 0) {
                                            //If the state is already in list.
                                            if (state < bracketContainer.size()) {
                                                double v = wcstod(bracketContainer.at(state).second, NULL) +
                                                           wcstod(left->value,NULL);
                                                wchar_t *buffer = (wchar_t *) malloc(
                                                        sizeof(left->value) * 10);
                                                swprintf(buffer, sizeof(left->value) * 10, L"%f", v);
                                                bracketContainer.insert(bracketContainer.begin() + state,
                                                                        pair<TokenType, wchar_t *>(DOUBLE, buffer));
                                                //If not.
                                            } else {
                                                bracketContainer.insert(bracketContainer.begin() + state,
                                                                        pair<TokenType, wchar_t *>(DOUBLE,
                                                                                                   left->value));
                                            }
                                        } else {
                                            valueContainer.push_back(
                                                    pair<TokenType, wchar_t *>(DOUBLE, left->value));
                                        }
                                    }
                                }
                            }
                        }
                        //TODO
                        //If type of any token is QUOTE.
                        /**
                        if (left->type == STRING || right->type == STRING) {
                            //Not reading in brackets
                            if (state <= 0) {
                                //Then + operator just concat number and string.
                                valueContainer.push_back(
                                        pair<TokenType, wchar_t *>(STRING, wcscat(left->value, right->value)));
                            } else {
                                bracketContainer.insert(bracketContainer.begin() + state - 1,
                                                        pair<TokenType, wchar_t *>(STRING,
                                                                                   wcscat(left->value, right->value)));
                            }
                            continue;
                        } else {
                            double leftValue = 0;
                            double rightValue = 0;
                            if (left->type == IDENTIFIER) {
                                if (valueExists(left->value))
                                    leftValue = findDoubleValue(left->value);
                                else {
                                    perror("Expected variable but not found");
                                    return;
                                }
                            } else {
                                leftValue = wcstod(left->value, NULL);
                            }
                            if (right->type == IDENTIFIER) {
                                if (valueExists(right->value))
                                    rightValue = findDoubleValue(right->value);
                                else {
                                    perror("Expected variable but not found");
                                    return;
                                }
                            } else {
                                rightValue = wcstod(right->value, NULL);
                            }
                            //Reading in brackets.
                            if (state > 0) {
                                //To convert double value to wchar_t, we make wchar_t buffer.
                                wchar_t *buffer = (wchar_t *) malloc(sizeof(left->value) + sizeof(right->value));
                                //Then convert.
                                swprintf(buffer, sizeof(left->value) + sizeof(right->value), L"%f",
                                         leftValue + rightValue);
                                //Push back to bracket container. Later, it sums up all data in it.
                                bracketContainer.push_back(pair<TokenType, wchar_t *>(DOUBLE, buffer));
                                //Not reading in brackets.
                            } else {
                                //To convert double value to wchar_t, we make wchar_t buffer.
                                wchar_t *buffer = (wchar_t *) malloc(sizeof(left->value) + sizeof(right->value));
                                //Then convert.
                                swprintf(buffer, sizeof(left->value) + sizeof(right->value), L"%f",
                                         leftValue + rightValue);
                                //Push back to value container. Later, it sums up all data in it.
                                valueContainer.push_back(pair<TokenType, wchar_t *>(DOUBLE, buffer));
                            }
                        }
                        */
                        break;
                    }
                    case SUB: {
                        if (state == 1) {

                        }
                        break;
                    }
                    case MUL: {
                        //An operator must have both side.
                        if (left == NULL || right == NULL) {
                            perror("An operator must have both side");
                            break;
                        }
                        //If type of any token is QUOTE.
                        if (left->type == END_QUOTE || right->type == START_QUOTE) {
                            //Multiplication does not support string.
                            perror("Cannot multiply string");
                            break;
                        }
                        //If statement is like " ~)*~"
                        if (left->type == END_BRACKET) {

                        }
                        //If statement is like "~*(~"
                        if (right->type == START_BRACKET) {

                            //If left token is not the end of bracket( like ) * ( )
                            if (left->type != END_BRACKET) {
                                //If left token type is identifier.
                                if (left->type == IDENTIFIER) {
                                    //If an identifier is not registered in symbol table
                                    if (!valueExists(left->value)) {
                                        //Then throw error
                                        throw invalid_argument("Expected variable but not found");
                                    }
                                    //If it is in brackets.
                                    if (state > 0) {
                                        //Then add to bracket container and set token type to
                                        //MUL to tell compiler to multiply this and next when summing up.

                                    }
                                } else {

                                }
                            }
                        }
                        //If a token is an identifier
                        if (left->type == IDENTIFIER) {
                            //Check if it is registered in symbol table.
                            if (valueExists(left->value)) {
                                leftValue = findDoubleValue(left->value);
                            }
                            //If a token is not an identifier, maybe just a number.
                        } else {
                            leftValue = wcstod(left->value, NULL);
                        }
                        //If a token is an identifier.
                        if (right->type == IDENTIFIER) {
                            //Check if it is registered in symbol table.
                            if (valueExists(right->value)) {
                                rightValue = findDoubleValue(right->value);
                            }
                            //If a token is not an identifier, maybe just a number.
                        } else {
                            rightValue = wcstod(right->value, NULL);
                        }
                        //And check if there are more MUL or DIV or MOD operator after.
                        Token *nextToken = j + 2 < tokenStream.size() ? &tokenStream.at(j + 2) : NULL;
                        if (nextToken != NULL &&
                            (nextToken->type == MUL || nextToken->type == DIV || nextToken->type == MOD)) {

                        }
                        break;
                    }
                    case DIV: {
                        break;
                    }
                    case START_BRACKET: {
                        state++;
                        break;
                    }
                    case END_BRACKET: {
                        if (state - 1 < 0) {
                            perror("The start bracket must have end bracket");
                            break;
                        }
                        //If the state exists in bracket container.
                        if (state < bracketContainer.size()) {
                            //If a bracket is in another bracket
                            if (state >= 1) {
                                //If the whole value of a bracket is multiplied by another.
                                //e.g. abc * ( ab + 123 )
                                //The token type of "abc" is MUL to notify that abc multiply with ( ab + 123)
                                if (bracketContainer.at(state - 1).first == MUL) {
                                    double value = wcstod(bracketContainer.at(state).second, NULL) *
                                                   wcstod(bracketContainer.at(state - 1).second, NULL);
                                    size_t sz = sizeof(bracketContainer.at(state).second) +
                                                sizeof(bracketContainer.at(state - 1).second);
                                    wchar_t *buffer = (wchar_t *) malloc(sz);
                                    swprintf(buffer, sz, L"%f", value);
                                    bracketContainer.erase(bracketContainer.begin() + state);
                                    if (nextToken->type == MUL || nextToken->type == DIV || nextToken->type == MOD) {
                                        bracketContainer.insert(bracketContainer.begin() + state - 1,
                                                                pair<TokenType, wchar_t *>(nextToken->type, buffer));
                                    }
                                } else if (bracketContainer.at(state - 1).first == DIV) {
                                    double value = wcstod(bracketContainer.at(state - 1).second, NULL) /
                                                   wcstod(bracketContainer.at(state).second, NULL);
                                    size_t sz = sizeof(bracketContainer.at(state).second) +
                                                sizeof(bracketContainer.at(state - 1).second);
                                    wchar_t *buffer = (wchar_t *) malloc(sz);
                                    swprintf(buffer, sz, L"%f", value);
                                    bracketContainer.erase(bracketContainer.begin() + state);
                                    if (nextToken->type == MUL || nextToken->type == DIV || nextToken->type == MOD) {
                                        bracketContainer.insert(bracketContainer.begin() + state - 1,
                                                                pair<TokenType, wchar_t *>(nextToken->type, buffer));
                                    }
                                } else if (bracketContainer.at(state - 1).first == MOD) {
                                    double value = ((int) wcstod(bracketContainer.at(state - 1).second, NULL) %
                                                    (int) wcstod(bracketContainer.at(state).second, NULL));
                                    size_t sz = sizeof(bracketContainer.at(state).second) +
                                                sizeof(bracketContainer.at(state - 1).second);
                                    wchar_t *buffer = (wchar_t *) malloc(sz);
                                    swprintf(buffer, sz, L"%f", value);
                                    bracketContainer.erase(bracketContainer.begin() + state);
                                    if (nextToken->type == MUL || nextToken->type == DIV || nextToken->type == MOD) {
                                        bracketContainer.insert(bracketContainer.begin() + state - 1,
                                                                pair<TokenType, wchar_t *>(nextToken->type, buffer));
                                    }
                                }
                            }
                        }
                        state--;

                    }
                }
            }
        }
    }
}


void clearBuffer(int *k, wchar_t *buffer, size_t size) {
    *k = 0;
    wmemset(buffer, 0, size);
}

void appendBuffer(int *k, wchar_t *buffer, wchar_t c) {
    buffer[*k] = c;
    *k = *k + 1;
}

wchar_t *typetostr(enum TokenType tokenType) {
    if (tokenType == IDENTIFIER)
        return L"IDENTIFIER";
    else if (tokenType == ADD)
        return L"ADD";
    else if (tokenType == SUB)
        return L"SUB";
    else if (tokenType == MUL)
        return L"MUL";
    else if (tokenType == DIV)
        return L"DIV";
    else if (tokenType == MOD)
        return L"MOD";
    else if (tokenType == IF)
        return L"IF";
    else if (tokenType == WHILE)
        return L"WHILE";
    else if (tokenType == ELSE)
        return L"ELSE";
    else if (tokenType == AND)
        return L"AND";
    else if (tokenType == OR)
        return L"OR";
    else if (tokenType == DOUBLE)
        return L"DOUBLE";
    else if (tokenType == INTEGER)
        return L"INTEGER";
    else if (tokenType == LONG)
        return L"LONG";
    else if (tokenType == FLOAT)
        return L"FLOAT";
    else if (tokenType == UNKNOWN)
        return L"UNKNOWN";
    else if (tokenType == ERROR)
        return L"ERROR";
    else if (tokenType == ASSIGN)
        return L"ASSIGN";
    else if (tokenType == EQUAL)
        return L"EQUAL";
    else if (tokenType == START_QUOTE)
        return L"START_QUOTE";
    else if (tokenType == END_QUOTE)
        return L"END_QUOTE";
    else if (tokenType == END_IF)
        return L"END_IF";
    else if (tokenType == END_WHILE)
        return L"END_WHILE";
}

bool isOperator(Token *token) {
    return token->type == ADD || token->type == SUB || token->type == MUL || token->type == DIV || token->type == AND ||
           token->type == OR;
}

bool valueExists(wchar_t *name) {
    return table.find(name) != table.end();
}

double findDoubleValue(wchar_t *name) {
    return wcstod(table[name], NULL);
}

wchar_t *findValue(wchar_t *name) {
    return table[name];
}

TokenType getType(wchar_t *name) {
    if (valueExists(name))
        return findType(table[name]);
    else
        return ERROR;
}

TokenType findType(wchar_t *value) {
    if (wcstod(value, NULL) != 0)
        return DOUBLE;
    else
        return STRING;
}