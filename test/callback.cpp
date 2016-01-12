#include <iostream>
#include <functional>
#include <future>
/*
template <class Fn, class... Args>
auto function_wrapper(Fn&& f , Args&&... args )->typename std::result_of<Fn(Args...)>::type
{
	using return_type = typename std::result_of<Fn(Args...)>::type;
	std::function< return_type()> functionp;
	functionp = std::bind(std::forward<Fn>(f), std::forward<Args>(args)...);
	
	return functionp();
}
*/

class CallBackBase
{
public:
	template <class Fn, class... Args>
	auto function_wrapper(Fn&& f , Args&&... args )->typename std::result_of<Fn(Args...)>::type
	{
		using return_type = typename std::result_of<Fn(Args...)>::type;
		std::function< return_type()> functionp;
		functionp = std::bind(std::forward<Fn>(f), std::forward<Args>(args)...);

//		return functionp();
		m_callbackfn = std::move(functionp);
	}
	void execute(void){
		m_callbackfn();
	}
private:
	std::function< void() > m_callbackfn;
};

void printhello(void){
	std::cout << "hello inside printhello" << std::endl;
}

void withparam(int x, int y){
	std::cout << "Hello with x " << x << " and y " << y <<std::endl;
}

int sum(int x, int y){
	return x+y;
}

class student
{
public:
	student(const std::string& name):m_name(name){}
	void print(void){
		std::cout << "Hello " << m_name << std::endl;
	}
	
private:
	std::string m_name;
};
int main(int argc, char *argv[]){
	CallBackBase cbObj;
	cbObj.function_wrapper(&printhello);
	cbObj.execute();
	cbObj.function_wrapper(&withparam, 10, 20);
	cbObj.execute();
	cbObj.function_wrapper(&sum, 10,5);
	std::cout << cbObj.execute() << std::endl;
/*
	student stobj("Rajesh");
	function_wrapper(&student::print,&stobj);
*/	return 0;
}
