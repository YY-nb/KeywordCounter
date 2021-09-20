#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>

//��Ϊ��С����Ϊ�˴����д�ļ���ֱ������ std
using namespace std;

const string keywords[] = {"auto","break","case","char","const","continue","default"
,"do","double","else","enum","extern","float","for","goto","if"
,"int","long","register","return","short","signed","sizeof","static"
,"struct","switch","typedef","union","unsigned","void","volatile","while"};

struct UserInput {
  string file_path;
  int level;
};
struct OutputData {
	int keyword_num;
	int switch_num;
	int case_num;
	int if_else_num;
	int if_elseif_else_num;
};
struct IgnoreList {
	bool double_slash; //   //
	bool slash_star; //     /* */
	bool quotes; //        ""
	bool macro;  //        #
	bool ignore_symbol_before; //��ǰ����ӵķ���֮ǰ��������ӵķ���
	int quote_time; // ���ų��ֵĴ���
};
//�û�������棬������ʾ���
class UserInterface {
  public:
	static void fillInput(UserInput *input); //�ȴ��û�����
	static void outputResult(OutputData *out);  //������
};

void UserInterface::fillInput(UserInput *input) {
  cout << "Please input the path of the code file:";
  cin >> input->file_path; 
  cout << "Please input the level:";
  cin >> input->level;
}
void UserInterface::outputResult(OutputData* out) {
  cout << out->keyword_num << endl;
}

//�����ļ�
class FileHandler {
  private:
	ifstream file;
	stringstream buffer;
	bool isFileOpenSuccess();
	  
  public:
    //�����������Ϣ��һ���ļ�
	void findFile(UserInput *input);
	string readFile();
	void closeStream();
	  	
};
void FileHandler::findFile(UserInput *input) {
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
  buffer << file.rdbuf(); //rdbuf()����ʵ��һ��������ָ�����������һ�������������
  return buffer.str();
}

class Counter {
  private:
	OutputData out;
	unordered_map<string, int> keyword_map;
	IgnoreList flags;
	bool isSymbolIgnore(const string &s, int i, IgnoreList *flags);
	void countKeyword(string s);
  public:
	Counter(const string arr[],int size);
	void startCount(string text, int level);
	OutputData* getOutput();
};
Counter::Counter(const string arr[], int size) {
  for (int i = 0; i < size; i++) {
	keyword_map.emplace(arr[i], 0); //Ч�ʱ� insert ��
  }
  flags = {false,false,false,false,false,0};
  out = { 0,0,0,0,0 };
}
bool Counter::isSymbolIgnore(const string &s,int i, IgnoreList *flags) {
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
  //�������űȽϸ��ӣ�����ǰ����ת���ַ���ǰ��û�������ú��Ե��ַ���ǰ���Ѿ���һ����ƥ�������
  if (s[i - 1] == '"' && s[i-2]!='\\' && (!flags->ignore_symbol_before || flags->quote_time == 1)) {
	flags->quotes = !flags->quotes; //�������������һ���ģ�����ֻ��ȡ��
    flags->quote_time = (flags->quote_time + 1) % 2; //�����ν��Ϊ1��ż���ν��Ϊ0
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
  return (flags->double_slash || flags->quotes || flags->slash_star);
}
void Counter::countKeyword(string s) {
  if (keyword_map.find(s)!=keyword_map.end()) {
	out.keyword_num++;
  }
}
void Counter::startCount(string text, int level) {
  int index = 0;
  string word;
  for (int i = 1; i < text.size(); i++) {
	if (isSymbolIgnore(text, i, &flags)) {
	  continue;
	}
	if (!isalpha(text[i - 1]) && isalpha(text[i])) {
	  index = i;
	}
	if (isalpha(text[i-1]) && !isalpha(text[i])) { //��ʱ����ȡ����
	  word = text.substr(index, i - index);
	  countKeyword(word);
	}
  }
}
OutputData* Counter::getOutput() {
  OutputData* out_ptr = &out;
  return out_ptr;
}
int main() {
  int arr_size = sizeof(keywords) / sizeof(keywords[0]);
  
  UserInput input;
  FileHandler handler;
  UserInterface::fillInput(&input);
  handler.findFile(&input); 
  string text=handler.readFile();
  handler.closeStream();
  Counter counter(keywords,arr_size);
  counter.startCount(text, input.level);
  UserInterface::outputResult(counter.getOutput());
  return 0;
  
 }

/*
 auto	break	case	char	const	continue	default	do
double	else	enum	extern	float	for	goto	if
int	long	register	return	short	signed	sizeof	static
struct	switch	typedef	union	unsigned	void	volatile	while
 */
//�������ǰ�����пո�
//�ֺ�֮ǰû�пո�
//������2���ո񣨹ȸ�淶��
//��ֵ�����ǰ�������пո�һԪ�����ǰ�޿ո��� ++x

