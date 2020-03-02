#include<iostream>

enum stat{
	a,
	b,
	c,
	d,
	e,
	f,
	g,
	h
}s;


int main(){
	std::cout << sizeof(s) << std::endl;
	switch(s){
		case a: std::cout << "this is a\n"; break;
		case b: std::cout << "this is b\n"; break;
		default : std::cout << "this is default\n";	
	}
	return 0;
}
