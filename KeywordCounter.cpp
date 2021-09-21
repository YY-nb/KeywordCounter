#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <stack>

//因为是小程序，为了代码编写的简便就直接引用 std
using namespace std;

const string keywords[] = { "auto","break","case","char","const","continue","default"
,"do","double","else","enum","extern","float","for","goto","if"
,"int","long","register","return","short","signed","sizeof","static"
,"struct","switch","typedef","union","unsigned","void","volatile","while" };

struct UserInput {
	string file_path;
	int level;
};
struct OutputData {
	int level;
	int keyword_num;
	int switch_num;
	int if_else_num;
	int if_elseif_else_num;
	vector<int> case_list;
};
struct IgnoreList {
	bool double_slash; //   //
	bool slash_star; //     /* */
	bool quotes; //        ""
	bool macro;  //        #
	bool ignore_symbol_before; //当前需忽视的符号之前有无需忽视的符号
	int quote_time; // 引号出现的次数
};

/******用户输入界面，并且显示结果******/
class UserInterface {
public:
	static void fillInput(UserInput* input); //等待用户输入
	static void outputResult(OutputData* out);  //输出结果
};

void UserInterface::fillInput(UserInput* input) {
	cout << "Please input the path of the code file:";
	cin >> input->file_path;
	cout << "Please input the level:";
	cin >> input->level;
}
void UserInterface::outputResult(OutputData* out) {
	if (out->level >= 1) {
		cout << "total num: " << out->keyword_num << endl;
	}
	if (out->level >= 2) {
		cout << "switch num: " << out->switch_num << endl;
		cout << "case num: ";
		if (out->case_list.empty()) {
			cout << 0 <<endl;
		}
		else
		{
			for (int i = 0; i < out->case_list.size(); i++) {
				cout << out->case_list[i] << " ";
			}
			cout << endl;
		}
		
	}
	if (out->level >= 3) {
		cout << "if-else num: " << out->if_else_num << endl;
	}
	if (out->level == 4) {
		cout << "if-elseif-else num: " << out->if_elseif_else_num << endl;
	}

}
/***************************/

/*********处理文件*********/
class FileHandler {
private:
	ifstream file;
	stringstream buffer;
	bool isFileOpenSuccess();

public:
	//根据输入的信息打开一个文件
	void findFile(UserInput* input);
	string readFile();
	void closeStream();

};
void FileHandler::findFile(UserInput* input) {
	file.open(input->file_path);
	while (!isFileOpenSuccess()) {
		cout << "\nThe file was not successfully opened."
			<< "\nPlease check if the file currently exists."
			<< endl;
		UserInterface::fillInput(input);
		file.open(input->file_path);
	}
}
bool FileHandler::isFileOpenSuccess() {
	if (file.fail()) {
		return false;
	}
	else {
		return true;
	}
}
void FileHandler::closeStream() {
	file.close();
}
string FileHandler::readFile() {
	buffer.str(""); // clear buffer first
	buffer << file.rdbuf(); //rdbuf()可以实现一个流对象指向的内容用另一个流对象来输出
	return buffer.str();
}
/***************************/

/*********关键字统计**********/
class Counter {
private:
	OutputData out;
	unordered_map<string, int> keyword_map;
	IgnoreList flags;
	stack<string> matching_stack;
	bool isSymbolIgnore(const string& s, int i, IgnoreList* flags);
	bool addInStack(string s, char* c);
	void addBracketInStack(char* c);
	void countKeyword(string s);
	void countSwitchCase(string s, int* case_list_index);
	void countIfElse(string s, char* c);
public:
	Counter(const string arr[], int size, int level);
	void startCount(string text, int level);
	OutputData* getOutput();
};
Counter::Counter(const string arr[], int size, int level) {
	for (int i = 0; i < size; i++) {
		keyword_map.emplace(arr[i], 0); //效率比 insert 快
	}
	flags = { false,false,false,false,false,0 };
	out = { level, 0,0,0,0 };
}
bool Counter::isSymbolIgnore(const string& s, int i, IgnoreList* flags) {
	if (s[i - 1] == '#' && !flags->ignore_symbol_before) {
		flags->macro = true;
		flags->ignore_symbol_before = true;
	}
	if (s[i - 1] == '/' && s[i] == '/' && !flags->ignore_symbol_before) {
		flags->double_slash = true;
		flags->ignore_symbol_before = true;
	}
	if (s[i - 1] == '/' && s[i] == '*' && !flags->ignore_symbol_before) {
		flags->slash_star = true;
		flags->ignore_symbol_before = true;
	}
	if (s[i - 1] == '*' && s[i] == '/' && flags->ignore_symbol_before) {
		flags->slash_star = false;
		flags->ignore_symbol_before = false;
	}
	//处理引号比较复杂，引号前避免转义字符，前面没有其他该忽略的字符或前面已经有一个相匹配的引号
	if (s[i - 1] == '"' && s[i - 2] != '\\' && (!flags->ignore_symbol_before || flags->quote_time == 1)) {
		flags->quotes = !flags->quotes; //计算机里引号是一样的，所以只能取反
		flags->quote_time = (flags->quote_time + 1) % 2; //奇数次结果为1，偶数次结果为0
		if (flags->quote_time == 0) {
			flags->ignore_symbol_before = false;
		}
		else
		{
			flags->ignore_symbol_before = true;
		}
	}

	if (s[i - 1] == '\n' && flags->double_slash) {
		flags->double_slash = false;
		flags->ignore_symbol_before = false;
	}
	if (s[i - 1] == '\n' && flags->macro) {
		flags->macro = false;
		flags->ignore_symbol_before = false;
	}
	return (flags->double_slash || flags->quotes || flags->slash_star || flags->macro);
}
void Counter::countKeyword(string s) {
	if (keyword_map.find(s) != keyword_map.end()) {
		out.keyword_num++;
	}
}
void Counter::countSwitchCase(string s, int* case_list_index) {
	if (s == "switch") {
		out.switch_num++;
		out.case_list.push_back(0);
		(*case_list_index)++;
	}
	else if (s == "case") {
		out.case_list[*case_list_index]++;
	}
}
void Counter::addBracketInStack(char* c) {
	string s(1, *c); // char to string
	if (*c == '{') {
		
		matching_stack.push(s);
	}
	if (*c == '}') {
		if (matching_stack.top() == "{") {
			matching_stack.pop();
		}
		else {
			matching_stack.push(s);
		}
	}
}
bool Counter::addInStack(string s, char* c) {
	if (s == "if") {
		char* p_temp = c - 3;
		while (*p_temp == ' ' || *p_temp == '\n') {
			p_temp--;
		}
		if (*p_temp == 'e') {
			matching_stack.push("elseif");
		}
		else  //此时是if
		{
			matching_stack.push(s);
		}
		return true;
	}
	if (s == "else") {
		char* p_temp = c;
		while (*p_temp == ' ' || *p_temp == '\n') {
			p_temp++;
		}
		if (*p_temp == 'i' && *(p_temp + 1) == 'f') {
			return true;
		}
		else    //此时是 else ，要准备与栈里的匹配
		{
			return false;
		}

	}
}
void Counter::countIfElse(string s, char* c) {
	if (!addInStack(s, c)) {  //此时是 else
		bool else_if = false;
		while (matching_stack.top() != "if") {
			if (matching_stack.top() == "}") {  //应对未消掉的{}，例如 if{if elseif} else
				do {
					matching_stack.pop();
				} while (matching_stack.top() != "{");
				matching_stack.pop(); //去掉 "{"
			}
			if (matching_stack.top() == "elseif") {
				else_if = true;
				matching_stack.pop();
			}
			

		}
		//此时栈顶是 if
		matching_stack.pop();
		if (else_if) {
			out.if_elseif_else_num++;
		}
		else {
			out.if_else_num++;
		}

	}

}
void Counter::startCount(string text, int level) {
	int index = 0, switch_num_temp = 0, case_list_index = -1;
	string word;

	for (int i = 1; i < text.size(); i++) {
		if (isSymbolIgnore(text, i, &flags)) {
			continue;
		}
		
		if (!isalpha(text[i - 1]) && isalpha(text[i])) {
			index = i;
		}
		if (isalpha(text[i - 1]) && !isalpha(text[i])) { //此时可提取单词
			word = text.substr(index, i - index);
			countSwitchCase(word, &case_list_index);
			countKeyword(word);
			countIfElse(word, &text[i]);
		}
		addBracketInStack(&text[i - 1]);
	}
}
OutputData* Counter::getOutput() {
	OutputData* p_out = &out;
	return p_out;
}
/***************************/

int main() {
	int arr_size = sizeof(keywords) / sizeof(keywords[0]);

	UserInput input;
	FileHandler handler;
	UserInterface::fillInput(&input);
	handler.findFile(&input);
	string text = handler.readFile();
	handler.closeStream();
	Counter counter(keywords, arr_size, input.level);
	counter.startCount(text, input.level);
	UserInterface::outputResult(counter.getOutput());
	return 0;

}


