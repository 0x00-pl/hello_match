#include<vector>
#include<sstream>
using namespace std;

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}


std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    return split(s, delim, elems);
}


namespace pl{

	typedef char* iter_type;
	typedef pair<iter_type,iter_type> iter_range;

	class rule{
	public:
		iter_range matched;
		virtual~rule(){}
		//state
		virtual bool next_match(iter_range src)=0;
		virtual rule* copy()=0;

		//static manager
		static rule* manager_get_rule(string s){
			return manager_reged[s]->copy();
		}
		static map<string, rule*> manager_reged;
	};
	
	class or_rule: public rule{
	public:
		// a | b 
		vector<string> rules;
		rule* matched_rule;
		or_rule(vector<string> rules){
			this->rules=rules;
			rules_index=0;
			matched_rule=manager_get_rule(rules[0]);
		}
		~or_rule(){
			clean_up();
		}
		void clean_up(){
			delete matched_rule;
		}
		//state
		size_t rules_index;
		virtual bool next_match(iter_range src){
			while( !matched_rule->next_match(src) ){
				//next rule
				clean_up();
				if(rules_index<rules.size())
					matched_rule=manager_get_rule(rules[++rules_index]);
				if(matched_rule==nullptr)
					return false;
			}
			matched=matched_rule->matched;
			return true;
		}
		virtual rule* copy(){return new or_rule(rules);}
	};
	
	class cons_rule: public rule{
	public:
		// a b 
		vector<string> rules;
		vector<rule*> matched_rule;
		cons_rule(vector<string> rules){
			this->rules=rules;
			for(size_t i=0; i< rules.size(); ++i)
				matched_rule[i]=manager_get_rule(rules[i]);
			first_match=true;
		}
		~cons_rule(){
			clean_up();
		}
		void clean_up(){
			//delete matched_rule;
		}
		//state
		bool first_match;
		virtual bool next_match(iter_range src){
			int stcp=matched_rule.size()-1;
			if(first_match){
				first_match=false;
				stcp=0;
			}
			while(0<stcp&&stcp<=matched_rule.size()){
				iter_range rg=src;
				if(stcp>1) rg.first=matched_rule[stcp-2]->matched.second;
				bool res=matched_rule[stcp-1]->next_match(src);
				if(res) ++stcp;
				else --stcp;
				//a b c *
				if(stcp>matched_rule.size()) return true;
			}
			matched.first= src.first;
			matched.second= matched_rule.back()->matched.second;
			return false;
		}
		virtual rule* copy(){return new cons_rule(rules);}
	};
}