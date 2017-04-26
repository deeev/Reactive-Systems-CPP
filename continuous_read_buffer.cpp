#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <unistd.h>

using namespace std;

template <typename Type_>
struct buffer {
    virtual ~buffer() {}

    virtual void put(Type_&) = 0;

    virtual Type_ get(void) = 0;
};

template <typename Type_>
class continuous_read_buffer : public buffer<Type_> {
public:
    explicit continuous_read_buffer(const Type_& default_val)
        : element_{default_val}
        , mtx_{}
    {
    }

    void put(Type_& element) override {
        lock_guard<mutex> lock__(mtx_);
        element_ = element;
    }

    Type_ get(void) override {
        lock_guard<mutex> lock__(mtx_);
        return element_;
    }

private:
    Type_ element_;
    mutex mtx_;
};

class Element {
public:
	Element(int i):i_(i){cout<<"Element with value "<< i_ << " created"<<endl;}
	~Element(){cout<<"Element with value "<< i_ << " destroyed"<<endl;}

private:
int i_;
};

class Consumer {
public:
	explicit Consumer(continuous_read_buffer<Element>& crb) : crb_(crb){}
	continuous_read_buffer<Element>& crb_;
	 void operator()() {
		
		while(true){
			usleep(250000);
			Element e = crb_.get();
		}
	}
};

class Producer {
public:
	explicit Producer(continuous_read_buffer<Element>& crb) : crb_(crb){}
	continuous_read_buffer<Element>& crb_;
	 void operator()() {
		int i = 0;
		while(true){
			usleep(500000);
			Element e(i);			
			crb_.put(e);
			i++;
		}
	}
};
int main() {

  	continuous_read_buffer<Element> crb(0);
	thread ConsumerThread( (Consumer(crb)) );
	thread ProducerThread( (Producer(crb)) );
	ProducerThread.join();
	ConsumerThread.join();

	

	return 0;
}
