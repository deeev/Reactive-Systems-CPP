#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>

template <typename Type_>
struct buffer {
    virtual ~buffer() {}

    virtual void put(Type_) = 0;

    virtual Type_ get(void) = 0;
};

template <typename Type_>
class full_synch_buffer : public buffer<std::unique_ptr<Type_>> {
public:
    full_synch_buffer()
        : element_{nullptr}
        , mtx_{}
        , cond_{}
    {
    }

    void put(std::unique_ptr<Type_> element) override {
        {
            std::unique_lock<std::mutex> lock__(mtx_);
            cond_.wait(lock__, [&]{return element_==nullptr;});
            element_ = std::move(element);
        }
        cond_.notify_one();
    }

    std::unique_ptr<Type_> get() override {
        decltype(element_) return__ = nullptr;
        {
            std::unique_lock<std::mutex> lock__(mtx_);
            cond_.wait(lock__, [&]{return element_!=nullptr;});
            return__ = std::move(element_);
        }
        cond_.notify_one();
        return return__;
    }

private:
    std::unique_ptr<Type_> element_;
    std::mutex mtx_;
    std::condition_variable cond_;
};


typedef int Element;

class Receiver {
public:
	explicit Receiver(full_synch_buffer<Element>& b) : b1(b){}
	full_synch_buffer<Element>& b1;// reference to channel
	 void operator()() {// this makes the class a functor. Put your event-loop in here.
		int read_value;
		for(int i=1; i<100; i++){
		//		std::this_thread::sleep_for(std::chrono::milliseconds(100));

			std::unique_ptr<int> r = b1.get();
            std::cout << "Get "<< *r <<" from Buffer"<<std::endl;

		}
	}
};

class Sender {
public:
	explicit Sender(full_synch_buffer<Element>& b) : b1(b){}
	full_synch_buffer<Element>& b1;//reference to a communication channel
	 void operator()() {
		for(int i=1; i<100; i++){
		//std::this_thread::sleep_for(std::chrono::milliseconds(100));
		std::unique_ptr<int> e(new int(i));
		b1.put(std::move(e));
		std::cout << "Put " <<i<<" into Buffer"<<std::endl;
		}
	}
};
int main() {
  full_synch_buffer<Element> fsb;
	std::thread ReceiverThread1( (Receiver(fsb)) );
	//	std::thread ReceiverThread2( (Receiver(fsb)) );

	std::thread SenderThread( (Sender(fsb)) );
	SenderThread.join();
	ReceiverThread1.join();
	//	ReceiverThread2.join();

	//fsb.destroy(); braucht man die? oder reicht der virtual
	//destructor, wenn der scope verlassen wird?

	return 0;
}
