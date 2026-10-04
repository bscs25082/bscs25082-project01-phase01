#include <iostream>
#include <string>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <limits>
#include <cctype>
#include <cstdio>
using namespace std;

//global consts
const int32_t maximumVariablesPerFrame = 16;
const int32_t maximumStackDepth = 64;
const int32_t maximumFunctions = 128;
const int32_t maximumTokens = maximumVariablesPerFrame + 2;
const int32_t maximumPatches = maximumFunctions * 4;
const uint64_t maximumSourceBytes = 15ULL * 1024 * 1024;
const int32_t ioBuffferSize = 64 * 1024;
const int32_t socketTimeOutSeconds = 5;

//stack class
template <typename temp>
class stackClass
{
    struct Node
    {
        temp dataNode;
        Node *nextNode;
    };
    Node *topNode;
    int32_t stackCount;
    
    public :
    //default constructor
    stackClass()
    {
        topNode = nullptr;
        stackCount = 0;
    }
    
    //deleted copy constructor
    stackClass(const stackClass&) = delete;
    
    //deleted assignment operator
    stackClass& operator=(const stackClass&) = delete;
    
    //destructor
    ~stackClass()
    {
        while(topNode != nullptr)
        {
            Node *tempNode = topNode;
            topNode = topNode->nextNode;
            delete tempNode;
        }
    }
    
    //function to push value
    void push(const temp &value)
    {
        if(stackCount == maximumStackDepth)
            throw overflow_error("maximum stack depth reached.");
        topNode = new Node{value, topNode};
        stackCount++;
    }
    
    //function to pop
    temp pop()
    {
        if(is_empty())
            throw underflow_error("stack is empty.");
        temp value = topNode->dataNode;
        Node *tempNode = topNode;
        topNode = topNode->nextNode;
        delete tempNode;
        stackCount--;
        return value;
    }
    
    //function for peek
    temp &peek()
    {
        if(is_empty())
            throw underflow_error("stack is empty.");
        return topNode->dataNode;
    }
    
    //function to check if its empty or not
    bool is_empty()
    {
        if(topNode == nullptr)
            return true;
        else
            return false;
    }
    
    //function to find depth
    int32_t depth()
    {
        return stackCount;
    }
    
    //function for snapshot
    int32_t snapshot_into(temp outStack[], int32_t maximumLength)
    {
        if(maximumLength < 0 || (maximumLength > 0 && outStack == nullptr))
            throw invalid_argument("invalid snapshot array.");
        Node *tempNode = topNode;
        int32_t writtenNodes = 0;
        while(tempNode != nullptr && writtenNodes < maximumLength)
        {
            outStack[writtenNodes++] = tempNode->dataNode;
            tempNode = tempNode->nextNode;
        }
        return writtenNodes;
    }
};

//snapshot struct
struct snapshot;

//struct for time line node
struct timeLineNode
{
    snapshot *dataNode;
    timeLineNode *nextNode;
    timeLineNode *previousNode;
};

//time line class
class timeLine
{
    private :
    timeLineNode *headNode;
    timeLineNode *tailNode;
    int32_t stepCount;
    
    public :
    //default constructor
    timeLine()
    {
        headNode = tailNode = nullptr;
        stepCount = 0;
    }
    
    //deleted copy constructr
    timeLine(const timeLine&) = delete;
    
    //deleted assignment operator
    timeLine& operator=(const timeLine&) = delete;
    
    //destructor
    ~timeLine();
    
    //function to record
    void record(snapshot *ss);
    
    //begin function
    timeLineNode *begin()
    {
        return headNode;
    }
    
    //function to get step count
    int32_t get_step_count()
    {
        return stepCount;
    }
};

//struct variable
struct variable
{
    string variableName;
    int32_t variableValue;
};

//frame struct
struct frame
{
    string functionName;
    int32_t argumentCount;
    variable argumentVector[maximumVariablesPerFrame];
    int32_t returnLine;
    variable locals[maximumVariablesPerFrame];
    int32_t localCount;
    variable *argumentRefernces[maximumVariablesPerFrame];
};

//struct of snapshot
struct snapshot
{
    frame callStack[maximumStackDepth];
    int32_t stackDepth;
};

//structor for TTDB header
struct TTDBHeader
{
    char magicArray[4];
    int32_t version;
    int32_t stepCount;
    int64_t indexOffset;
};

//timeline class destructor
timeLine::~timeLine()
{
    while(headNode != nullptr)
    {
        timeLineNode *tempNode = headNode;
        headNode = headNode->nextNode;
        delete tempNode->dataNode;
        delete tempNode;
    }
}

//record function from time line class
void timeLine::record(snapshot *ss)
{
    if(ss == nullptr)
        throw invalid_argument("null snapshot.");
    if(stepCount == numeric_limits<int32_t>::max())
    {
        delete ss;
        throw overflow_error("too many snapshots.");
    }
    timeLineNode *tempNode;
    try
    {
        tempNode = new timeLineNode{ss, nullptr, tailNode};
    }
    catch(...)
    {
        delete ss;
        throw ;
    }
    if(headNode == nullptr)
        headNode = tempNode;
    else
        tailNode->nextNode = tempNode;
    tailNode = tempNode;
    stepCount++;
}

//functin to write bytes
void write_bytes(FILE *file, const void *dataNode, size_t size)
{
    if(size > 0 && fwrite(dataNode, 1, size, file) != size)
        throw runtime_error("couldnt write file.");
}

//function to read bytes
void read_bytes(FILE *file, void *dataNode, size_t size)
{
    if(size > 0 && fread(dataNode, 1, size, file) != size)
        throw runtime_error("incomplerte/unreadable binary record.");
}

//function to return file position
int64_t file_position(FILE *file)
{
    long position = ftell(file);
    if(position < 0)
        throw runtime_error("coulnt read file position.");
    return static_cast<int64_t>(position);
}

//function to seek file
void seek_file(FILE *file, int64_t position)
{
    if(position < 0 || position > numeric_limits<long>::max() || fseek(file, static_cast<long>(position), SEEK_SET) != 0)
        throw runtime_error("couldnt seek in file.");
}

//function to close file
void close_file(FILE *&file)
{
    FILE *tempNode = file;
    file = nullptr;
    if(fclose(tempNode) != 0)
        throw runtime_error("couldnt close file.");
}

//function to write header
void write_header(FILE *file, const TTDBHeader &header)
{
    write_bytes(file, header.magicArray, 4);
    write_bytes(file, &header.version, sizeof(int32_t));
    write_bytes(file, &header.stepCount, sizeof(int32_t));
    write_bytes(file, &header.indexOffset, sizeof(int64_t));
}

//function entry struct
struct functionEntry
{
    string functionName;
    int64_t byteOffsetInResolveBin;
};

//pending patch struct
struct pendingPatch
{
    int64_t byteOffsetOfOffsetField;
    string targetFunctionName;
};

//function to read source line
bool read_source_line(ifstream &fin, string &outString)
{
    while(getline(fin, outString))
        if(outString.find_first_not_of(" \t\r\n") != string::npos)
            return true;
    return false;
}

//function for first word
string first_word(const string &line)
{
    istringstream sin(line.substr(0, line.find("//")));
    string firstWord;
    sin >> firstWord;
    return firstWord;
}

//function to get second word
string second_word(const string &line)
{
    istringstream sin(line.substr(0, line.find("//")));
    string firstWord, secondWord;
    sin >> firstWord >> secondWord;
    return secondWord;
}

//function to check if its identifier
bool is_identifier(const string &name)
{
    if(name.empty())
        return false;
    if(!isalpha(static_cast<unsigned char>(name[0])) && name[0] != '_')
        return false;
    for(size_t p = 1; p < name.size(); p++)
        if(!isalnum(static_cast<unsigned char>(name[p])) && name[p] != '_')
            return false;
    return true;
}

//function to validate program
bool validate_program(const char *sourcePath)
{
    try
    {
        ifstream fin(sourcePath, ios::binary);
        if(!fin)
            throw runtime_error("cannot open source.bin.");
        fin.seekg(0, ios::end);
        streamoff size = fin.tellg();
        if(size < 0 || static_cast<uint64_t>(size) > maximumSourceBytes)
            throw runtime_error("source file exceeds 15 Mib or is unreadable.");
        fin.seekg(0, ios::beg);
        stackClass<string> functions;
        string line;
        while(read_source_line(fin, line))
        {
            string firstWord = first_word(line);
            if(firstWord.empty())
                continue;
            if(firstWord == "func")
            {
                if(!functions.is_empty())
                    throw runtime_error("nested function declaration arent allowed.");
                string name = second_word(line);
                if(!is_identifier(name))
                    throw runtime_error("invalid function name.");
                functions.push(name);
            }
            else if(firstWord == "func_end")
            {
                if(functions.is_empty())
                    throw runtime_error("unmatched func_end.");
                if(!second_word(line).empty())
                    throw runtime_error("func_end takes no arguments.");
                functions.pop();
            }
            else if(functions.is_empty())
                throw runtime_error("instructions outside function.");
        }
        if(fin.bad())
            throw runtime_error("couldnt read source.bin.");
        if(!functions.is_empty())
            throw runtime_error("missing func_end.");
        return true;
    }
    catch(const exception &e)
    {
        cerr << "validation error : " << e.what() << '\n';
        return false;
    }
}

//function to write resolve record
int64_t write_resolve_record(FILE *file, int64_t offsetField, const string &text)
{
    if(text.size() > maximumSourceBytes)
        throw runtime_error("source line too long.");
    int64_t start = file_position(file);
    int32_t size = static_cast<int32_t>(text.size());
    write_bytes(file, &offsetField, sizeof(int64_t));
    write_bytes(file, &size, sizeof(int32_t));
    write_bytes(file, text.data(), text.size());
    return start;
}

//functin to read resolve record
int64_t read_resolve_record(FILE *file, string &outText)
{
    int64_t offset;
    size_t bytes = fread(&offset, 1, sizeof(int64_t), file);
    if(bytes == 0 && feof(file))
    {
        outText.clear();
        return -1;
    }
    if(bytes != sizeof(int64_t)) throw runtime_error("incomplete resolve offset.");
    int32_t size;
    read_bytes(file, &size, sizeof(int32_t));
    if(offset < 0 || size < 0 || static_cast<uint64_t>(size) > maximumSourceBytes)
        throw runtime_error("invalid resolve record.");
    outText.resize(size);
    if(size > 0)
        read_bytes(file, &outText[0], size);
    return offset;
}

int64_t resolve_program(const char *sourcePath, const char *resolveBinPath)
{
    functionEntry functionArray[maximumFunctions];
    int32_t functionCount = 0;
    pendingPatch patches[maximumPatches];
    int32_t patchesCount = 0;
    int64_t mainOffset = -1;
    ifstream fin(sourcePath, ios::binary);
    if(!fin)
        throw runtime_error("cannot open source.bin.");
    FILE *fout = fopen(resolveBinPath, "wb+");
    if(fout == nullptr)
        throw runtime_error("cannot create resolve.bin.");
    try
    {
        string line;
        while(getline(fin, line))
        {
            int64_t position = file_position(fout);
            write_resolve_record(fout, position, line);
            string firstWord = first_word(line);
            if(firstWord == "func")
            {
                string name = second_word(line);
                if(!is_identifier(name))
                    throw runtime_error("invalid function name.");
                if(functionCount == maximumFunctions)
                    throw runtime_error("too much functions.");
                for(int32_t p = 0; p < functionCount; p++)
                    if(functionArray[p].functionName == name)
                        throw runtime_error("duplicate function : " + name);
                functionArray[functionCount++] = functionEntry{name, position};
                if(name == "main")
                    mainOffset = position;
            }
            else if(firstWord == "call")
            {
                string target = second_word(line);
                if(!is_identifier(target))
                    throw runtime_error("invalud call target.");
                if(patchesCount == maximumPatches)
                    throw runtime_error("too many call records.");
                patches[patchesCount++] = pendingPatch{position, target};
            }
        }
        if(fin.bad())
            throw runtime_error("couldnt read source.bin.");
        if(mainOffset < 0)
            throw runtime_error("function main wasnt found.");
        for(int32_t p = 0; p < patchesCount; p++)
        {
            int64_t target = -1;
            for(int32_t q = 0; q < functionCount; q++)
            {
                if(functionArray[q].functionName == patches[p].targetFunctionName)
                {
                    target = functionArray[q].byteOffsetInResolveBin;
                    break;
                }
            }
            if(target < 0)
                throw runtime_error("undefined function : " + patches[p].targetFunctionName);
            seek_file(fout, patches[p].byteOffsetOfOffsetField);
            write_bytes(fout, &target, sizeof(int64_t));
        }
        close_file(fout);
        return mainOffset;
    }
    catch(...)
    {
        if(fout != nullptr)
            fclose(fout);
        throw;
    }
}

//token type
enum tokenType
{
    KEYWORD, IDENTIFIER, PARAM
};

//token struct
struct token
{
    tokenType type;
    string text;
};

//function to tokenize line
int32_t tokenize_line(const string &line, token tokens[], int32_t maximumTokens)
{
    istringstream fin(line.substr(0, line.find("//")));
    string word;
    int32_t tokenCount = 0;
    while(fin >> word)
    {
        if(tokenCount >= maximumTokens)
            throw runtime_error("too many tokens on line.");
        tokens[tokenCount].type = tokenCount == 0? KEYWORD : (tokenCount == 1? IDENTIFIER : PARAM);
        tokens[tokenCount].text = word;
        tokenCount++;
    }
    return tokenCount;
}

//function to find variable
variable *find_variable(frame &frameVariable, const string &name)
{
    for(int32_t p = 0; p < frameVariable.argumentCount; p++)
        if(frameVariable.argumentVector[p].variableName == name)
            return frameVariable.argumentRefernces[p] == nullptr? &frameVariable.argumentVector[p] : frameVariable.argumentRefernces[p];
    for(int32_t p = 0; p < frameVariable.localCount; p++)
        if(frameVariable.locals[p].variableName == name)
            return &frameVariable.locals[p];
    return nullptr;
}

//function to read value
int32_t read_value(frame &frameVariable, const string &text)
{
    if(is_identifier(text))
    {
        variable *variablePtr = find_variable(frameVariable, text);
        if(variablePtr == nullptr)
            throw runtime_error("undefined variable : " + text);
        return variablePtr->variableValue;
    }
    size_t used= 0;
    long long value;
    try
    {
        value = stoll(text, &used, 10);
    }
    catch(const exception&)
    {
        throw runtime_error("invalid integer : " + text);
    }
    if(used != text.size() || value < numeric_limits<int32_t>::min() || value > numeric_limits<int32_t>::max())
        throw runtime_error("invalid 32-bit integer : " + text);
    return static_cast<int32_t>(value);
}

//function to make frame
frame make_frame(token tokens[], int32_t tokenCount, int32_t returnLine)
{
    if(tokenCount < 2 || tokens[0].text != "func" || !is_identifier(tokens[1].text))
        throw runtime_error("invalid function header.");
    frame frameVariable{};
    frameVariable.functionName = tokens[1].text;
    frameVariable.argumentCount = tokenCount - 2;
    frameVariable.returnLine = returnLine;
    for(int32_t p = 0; p < frameVariable.argumentCount; p++)
    {
        string name = tokens[p + 2].text;
        if(!is_identifier(name))
            throw runtime_error("invalid parameter : " + name);
        for(int32_t q = 0; q < p; q++)
            if(frameVariable.argumentVector[q].variableName == name)
                throw runtime_error("duplicate parameter : " + name);
        frameVariable.argumentVector[p].variableName = name;
    }
    return frameVariable;
}

//function to build snapshot
snapshot *build_snapshot(stackClass<frame> &callStack)
{
    snapshot *ss = new snapshot();
    try
    {
        ss->stackDepth = callStack.snapshot_into(ss->callStack, maximumStackDepth);
        for(int32_t p = 0; p < ss->stackDepth; p++)
        {
            frame &frameVariable = ss->callStack[p];
            for(int32_t q = 0; q < frameVariable.argumentCount; q++)
            {
                if(frameVariable.argumentRefernces[q] != nullptr)
                    frameVariable.argumentVector[q].variableValue = frameVariable.argumentRefernces[q]->variableValue;
                frameVariable.argumentRefernces[q] = nullptr;
            }
        }
        return ss;
    }
    catch(...)
    {
        delete ss;
        throw;
    }
}

//function to excute program
void excute_program(const char *resolveBinPath, int64_t mainOffset, timeLine &timelineNode)
{
    FILE *fin = fopen(resolveBinPath, "rb");
    if(fin == nullptr)
        throw runtime_error("cannot open resolve.bin.");
    try
    {
        stackClass<frame> callStack;
        string line;
        token tokens[maximumTokens];
        seek_file(fin, mainOffset);
        if(read_resolve_record(fin, line) < 0)
            throw runtime_error("missing main record.");
        int32_t tokenCount = tokenize_line(line, tokens, maximumTokens);
        frame mainFrame = make_frame(tokens, tokenCount, -1);
        if(mainFrame.functionName != "main" || mainFrame.argumentCount != 0)
            throw runtime_error("entry function must be : func main.");
        callStack.push(mainFrame);
        seek_file(fin, mainOffset);
        while(!callStack.is_empty())
        {
            int64_t targetOffset = read_resolve_record(fin, line);
            if(targetOffset < 0)
                throw runtime_error("unexpceted end of function.");
            tokenCount = tokenize_line(line, tokens, maximumTokens);
            if(tokenCount == 0)
                continue;
            string word = tokens[0].text;
            if(word == "func")
            {
                frame header = make_frame(tokens, tokenCount, 0);
                if(header.functionName != callStack.peek().functionName)
                    throw runtime_error("unexpected function declaration.");
            }
            else if(word == "func_end")
            {
                if(tokenCount != 1)
                    throw runtime_error("func_end takes no arguments.");
                int32_t returnLine = callStack.peek().returnLine;
                callStack.pop();
                if(!callStack.is_empty())
                    seek_file(fin, returnLine);
            }
            else if(word == "call")
            {
                if(tokenCount < 2)
                    throw runtime_error("CALL needs a function name.");
                int64_t returnPosition = file_position(fin);
                if(returnPosition > numeric_limits<int32_t>::max())
                    throw runtime_error("return address exceeds frame limit.");
                seek_file(fin, targetOffset);
                string headerLine;
                if(read_resolve_record(fin, headerLine) < 0)
                    throw runtime_error("missing CALL target record.");
                token headerTokens[maximumTokens];
                int32_t headerCount = tokenize_line(headerLine, headerTokens, maximumTokens);
                frame nextFrame = make_frame(headerTokens, headerCount, static_cast<int32_t>(returnPosition));
                if(nextFrame.functionName != tokens[1].text || nextFrame.argumentCount != tokenCount - 2)
                    throw runtime_error("CALL argument count or target mismatch : " + tokens[1].text);
                frame &callerFrame = callStack.peek();
                for(int32_t p = 0; p < nextFrame.argumentCount; p++)
                {
                    string argument = tokens[p + 2].text;
                    nextFrame.argumentVector[p].variableValue = read_value(callerFrame, argument);
                    if(is_identifier(argument)) nextFrame.argumentRefernces[p] = find_variable(callerFrame, argument);
                }
                callStack.push(nextFrame);
                seek_file(fin, targetOffset);
            }
            else if(word == "set" || word == "add" || word == "sub" || word == "mul" || word == "div")
            {
                if(tokenCount != 3 || !is_identifier(tokens[1].text))
                    throw runtime_error("expected : " + word + "variable operand");
                frame &frameVariable = callStack.peek();
                int32_t value = read_value(frameVariable, tokens[2].text);
                variable *variableOne = find_variable(frameVariable, tokens[1].text);
                if(word == "set")
                {
                    if(variableOne == nullptr)
                    {
                        if(frameVariable.localCount == maximumVariablesPerFrame)
                            throw runtime_error("too many local variables.");
                        variableOne = &frameVariable.locals[frameVariable.localCount++];
                        variableOne->variableName = tokens[1].text;
                    }
                    variableOne->variableValue = value;
                }
                else
                {
                    if(variableOne == nullptr)
                        throw runtime_error("undefined variable : " + tokens[1].text);
                    int64_t resultVariable = variableOne->variableValue;
                    if(word == "add")
                        resultVariable = resultVariable + value;
                    else if(word == "sub")
                        resultVariable = resultVariable - value;
                    else if(word == "mul")
                        resultVariable = resultVariable * value;
                    else
                    {
                        if(value == 0)
                            throw runtime_error("division by zero.");
                        resultVariable = resultVariable / value;
                    }
                    if(resultVariable < numeric_limits<int32_t>::min() || resultVariable > numeric_limits<int32_t>::max())
                        throw overflow_error("32-bit arithmetic overflow.");
                    variableOne->variableValue = static_cast<int32_t>(resultVariable);
                }
            }
            else throw runtime_error("unknown instructions : " + word);
            timelineNode.record(build_snapshot(callStack));
        }
        close_file(fin);
    }
    catch(...)
    {
        if(fin != nullptr)
            fclose(fin);
        throw;
    }
}

//functin to write a string
void write_string(FILE *file, const string &text)
{
    if(text.size() > static_cast<size_t>(numeric_limits<int32_t>::max()))
        throw runtime_error("string too long.");
    int32_t stringSize = static_cast<int32_t>(text.size());
    write_bytes(file, &stringSize, sizeof(int32_t));
    write_bytes(file, text.data(), text.size());
}

//function to write variable
void write_variable(FILE *file, const variable &variableOne)
{
    write_string(file, variableOne.variableName);
    write_bytes(file, &variableOne.variableValue, sizeof(int32_t));
}

//function to write tdbg
void write_time_travel_debugger(timeLine &timeline, const char *timeTravelDebuggerPath)
{
    FILE *fout = fopen(timeTravelDebuggerPath, "wb");
    if(fout == nullptr)
        throw runtime_error("cannot create session.tdbg");
    int64_t *TTDBindex = nullptr;
    try
    {
        TTDBHeader header = {{'T', 'T', 'D', 'B'}, 1, timeline.get_step_count(), 0};
        write_header(fout, header);
        TTDBindex = new int64_t[header.stepCount];
        timeLineNode *timelineNode = timeline.begin();
        for(int32_t stepCount = 0; stepCount < header.stepCount; stepCount++)
        {
            if(timelineNode == nullptr)
                throw runtime_error("Incomplete timeline");
            TTDBindex[stepCount] = file_position(fout);
            const snapshot &ss = *timelineNode->dataNode;
            write_bytes(fout, &ss.stackDepth, sizeof(int32_t));
            for(int32_t p = 0; p < ss.stackDepth; p++)
            {
                const frame &frameVariable = ss.callStack[p];
                write_string(fout, frameVariable.functionName);
                write_bytes(fout, &frameVariable.argumentCount, sizeof(int32_t));
                for(int32_t q = 0; q < frameVariable.argumentCount; q++)
                    write_variable(fout, frameVariable.argumentVector[q]);
                write_bytes(fout, &frameVariable.returnLine, sizeof(int32_t));
                write_bytes(fout, &frameVariable.localCount, sizeof(int32_t));
                for(int32_t q = 0; q < frameVariable.localCount; q++) write_variable(fout, frameVariable.locals[q]);
            }
            timelineNode = timelineNode->nextNode;
        }
        header.indexOffset = file_position(fout);
        write_bytes(fout, TTDBindex, sizeof(int64_t) * static_cast<size_t>(header.stepCount));
        seek_file(fout, 0);
        write_header(fout, header);
        close_file(fout);
        delete[] TTDBindex;
    }
    catch(...)
    {
        delete[] TTDBindex;
        if(fout != nullptr)
            fclose(fout);
        throw;
    }
}

int32_t main()
{
    
    if(!validate_program("source.bin"))
        return -1;
    try
    {
        int64_t mainOffset = resolve_program("source.bin", "resolve.bin");
        timeLine timeline;
        excute_program("resolve.bin", mainOffset, timeline);
        write_time_travel_debugger(timeline, "session.tdbg");
        cout << "created session.tdbg with " << timeline.get_step_count() << " snapshots." << endl;
        return 0;
    }
    catch(const exception &e)
    {
        cerr << "error : " << e.what() << '\n';
        return 1;
    }
}
