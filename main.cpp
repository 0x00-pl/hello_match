#include<iostream>
#include<string>
#include<vector>
#include<map>
//#include"iter_match.h"
#include"m1.h"
using namespace std;
/*
template<typename T>
class auto_del{
public:
	T* ptr;
	auto_del(T* _ptr){ptr=_ptr;}
	~auto_del(){delete ptr;}
	T* operator->(){return ptr;}
	T& operator*(){return *ptr;}
};

class charEX{
public:
	charEX(char* pos){
		if(*pos=='\\'){
			++pos;
			quote=true;
		}
		value=*pos;
	}
	bool operator==(charEX other){
		return 
			value==other.value &&
			quote==other.quote ;
	}
	bool operator==(char* other){
		if(*other=='\\')return value==other[1];
		return value==*other;
	}
	operator char(){
		return value;
	}
	size_t size(){return quote? 2: 1;}
	bool quote;
	char value;
};

class char_iter{
public:
	char* val;
	char_iter(){}
	char_iter(char* v){val=v;}
	operator char*(){return val;}
	charEX operator*(){return charEX(val);}
	bool operator==(char_iter dest){return *val==*dest;}
	char_iter& operator++(){
		if(val=='\0') return *this;
		val+= charEX(val).size();
		return *this;
	}
};


typedef char_iter Titer;

bool is_space(Titer pos){
	return
		*pos==' ' ||
		*pos=='\n' ||
		*pos=='\t' ||
		*pos=='\r' ;
}
bool is_bracket(Titer pos){
	return *pos==')' || *pos==")";
}


Titer end_space(Titer beg){
	while(is_space(beg)) ++beg;
	return beg;
}

Titer end_element(Titer beg){
	//if not %element EX%
	while(!is_space(beg) && !is_bracket(beg)) ++beg;
	return beg;
}

//skip space and get element
pair<Titer,Titer> get_element(Titer beg){
	beg= end_space(beg);
	pair<Titer,Titer> ret;
	ret.first= beg;
	ret.second= end_element(beg);
	return ret;
}

typedef char pl_stream_element;
class pl_stream{
public:
	class stream_pos{public:virtual~stream_pos(){}};
	typedef pl_stream_element element_type;
	virtual~pl_stream(){}
	virtual bool in(pl_stream_element e)=0;//{return false;}
	virtual bool out(pl_stream_element& e)=0;//{return false;}
	virtual stream_pos* get_in_pos()=0;//{return new stream_pos();}
	virtual bool set_in_pos(stream_pos*)=0;//{return false;}//return true if success
	virtual stream_pos* get_out_pos()=0;//{return new stream_pos();}
	virtual bool set_out_pos(stream_pos*)=0;//{return false;}//return true if success
};

class pl_stream_char_buff:public pl_stream{
public:
	class stream_pos:public pl_stream::stream_pos{public: int pos;};
	typedef char& element_type;

	char* buff;
	int max;
	int pin;
	int pout;
	pl_stream_char_buff(int size){
		buff=new char[size];
		pin=pout=0;
		max=size;
	}
	virtual bool in(pl_stream_element e){
		if(pin>=max) return false;
		buff[pin]=e;
		pin++;
		return true;
	}
	virtual bool out(pl_stream_element& e){
		if(pout>=pin) return false;
		e= buff[pout];
		pout++;
		return true;
	}
	virtual stream_pos* get_in_pos(){
		stream_pos* ret=new stream_pos();
		ret->pos= pin;
		return ret;
	}
	virtual bool set_in_pos(pl_stream::stream_pos* pos){
		stream_pos* p= dynamic_cast<stream_pos*>(pos);
		if(p->pos< pout)return false;
		pin= p->pos;
		return true;
	}//return true if success
	virtual stream_pos* get_out_pos(){
		stream_pos* ret=new stream_pos();
		ret->pos= pout;
		return ret;
	}
	virtual bool set_out_pos(pl_stream::stream_pos* pos){
		stream_pos* p= dynamic_cast<stream_pos*>(pos);
		if(p->pos> pin)return false;
		pout= p->pos;
		return true;
	}//return true if success
};

class pl_match{
public:
	virtual~pl_match(){}
	virtual bool in(pl_stream& src){return false;}
	virtual bool out(pl_stream& dst){return false;}
	virtual pl_match* copy()=0;
	virtual bool operator==(const pl_match&)=0;
};


class pl_match_const_string:public pl_match{
public:
	string value;
	pl_match_const_string(string s){
		value=s;
	}
	
	virtual bool in(pl_stream& src){
		for(int i=0; i<value.length(); ++i){
			pl_stream::element_type e;
			if(!src.out(e)) return false;
			if(e!=value.at(i)) return false;
		}
		return true;
	}
	virtual bool out(pl_stream& dst){
		for(int i=0; i<value.length(); ++i){
			dst.in(value.at(i));
		}
		return true;
	}
	virtual pl_match* copy(){return new pl_match_const_string(*this);}
};

class pl_match_data_bloc:public pl_match{
public:
	pl_stream::element_type* buff;
	int buf_size;
	pl_match_data_bloc(int size){buf_size=size;buff=new pl_stream::element_type[size];}
	~pl_match_data_bloc(){delete buff;}
	
	virtual bool in(pl_stream& src){
		for(int i=0; i<buf_size; ++i){
			pl_stream::element_type e;
			if(!src.out(e)) return false;
			buff[i]=e;
		}
		return true;
	}
	virtual bool out(pl_stream& dst){
		for(int i=0; i<buf_size; ++i){
			dst.in(buff[i]);
		}
		return true;
	}
	virtual pl_match* copy(){return new pl_match_data_bloc(*this);}
};

class pl_match_cons_match:public pl_match{
public:
	pl_match& ma;
	pl_match& mb;

	pl_match_cons_match(pl_match& first, pl_match& second):ma(first),mb(second){
		
	}
	virtual bool in(pl_stream& src){
		auto_del<pl_stream::stream_pos> beg(src.get_out_pos());

		if(!ma.in(src)){ 
			src.set_out_pos(&*beg);
			return false;
		}
		if(!mb.in(src)) { 
			src.set_out_pos(&*beg);
			return false;
		}
		return true;
	}
	virtual bool out(pl_stream& dst){	
		auto_del<pl_stream::stream_pos> beg(dst.get_in_pos());

		if(!ma.out(dst)){ 
			dst.set_in_pos(&*beg);
			return false;
		}
		if(!mb.out(dst)) { 
			dst.set_in_pos(&*beg);
			return false;
		}
		return true;
	}
	virtual pl_match* copy(){return new pl_match_cons_match(*this);}
};
class pl_match_while_until:public pl_match{
public:
	vector<pl_match*> matched;
	pl_match& protype;
	pl_match_while_until(pl_match& until):protype(until){}
	~pl_match_while_until(){
		for(int i=0; i<matched.size(); i++){
			delete matched[i];
		}
	}
	
	virtual bool in(pl_stream& src){
		auto_del<pl_stream::stream_pos> beg(src.get_out_pos());
		while(1){ 
			pl_match* test=protype.copy();
			if(!test->in(src)){
				src.set_out_pos(&*beg);
				return false;
			}
			matched.push_back(test);
			if(*test==protype) return true;
		}
	}
	virtual bool out(pl_stream& dst){
		auto_del<pl_stream::stream_pos> beg(dst.get_in_pos());
		for(int i=0; i<matched.size(); i++){
			if(!matched[i]->out(dst)){
				dst.set_in_pos(&*beg);
				return false;
			}
		}
		return true;
	}
	virtual pl_match* copy()=0;
};

*/
void main(){
	/*
	pl_stream_char_buff ss(100);
	for(int i=0; i<100; ++i) ss.in('0');

	//Æ¥ÅäÈÎÒâ3¸ö×Ö·û
	pl_match_data_bloc md(3);
	//Æ¥Åä000
	pl_match_const_string mcs("000");

	//Æ¥Åä ÈÎÒâ3×Ö·û+000
	pl_match_cons_match mcm(mcs, md);

	//ÊäÈë×Ö·û´®Á÷ ¿ªÊ¼Æ¥Åä
	mcm.in(ss);
	*/

}