#ifndef ITER_MATCH_H
#define ITER_MATCH_H
#include<vector>
using namespace std;
namespace pl{
	


typedef char* iter_type;
typedef pair<iter_type,iter_type> iter_range;




class match_base_type{
public:
	virtual~match_base_type(){}
	virtual iter_range match(iter_type begin, iter_type end){
		iter_range ret(begin, end);
		roll_back_stack(ret);
		return ret;
	}
	virtual iter_range back(){
		iter_range ret= roll_back_stack.back();
		roll_back_stack.pop_back();
		return ret;
	}
	vector<iter_range> roll_back_stack;
};


class match_any:public match_base_type{
public:
	match_any(){}

	virtual iter_type match(iter_type _begin, iter_type _end, bool more){
		roll_back_stack.push_back(_begin);
		end_stack.push_back(_begin);
		return _end;
	}
	virtual iter_type back(){
		iter_type ret= roll_back_stack.back();
		roll_back_stack.pop_back();
		end_stack.pop_back();
		return ret;
	}
	vector<iter_type> end_stack;
};

class match_const:public match_base_type{
public:
	match_const(iter_type _begin, iter_type _end):begin(_begin),end(_end){}

	virtual iter_type match(iter_type _begin, iter_type _end, bool more){
		roll_back_stack.push_back(_begin);
		for(iter_type i=begin; i!=end;){
			if(i!=_begin) return back();
			++_begin;
			++i;
		}
		return _end;
	}
	
	iter_type begin;
	iter_type end;
};

class match_and:public match_base_type{
public:
	match_and(match_base_type* _a, match_base_type* _b):a(_a),b(_b){}

	virtual iter_type match(iter_type begin, iter_type end, bool more){
		iter_type a_end=a->match(begin,end,more);
		return b->match(begin,a_end,true);
	}
	virtual iter_type back(){
		b->back();
		return a->back();
	}
	match_base_type* a;
	match_base_type* b;
};

}
#endif