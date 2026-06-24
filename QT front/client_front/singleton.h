#ifndef SINGLETON_H
#define SINGLETON_H
#include<memory>
#include<mutex>
#include<iostream>
using namespace std;
template<typename T>
class Singleton{
protected:
    Singleton()=default;
    Singleton(const Singleton<T>&) = delete;
    Singleton& operator=(const Singleton<T>& st)=delete;
    static std::shared_ptr<T> instance_;
public:
    static std::shared_ptr<T> GetInstance(){
        static std::once_flag s_flag;
        std::call_once(s_flag, [&]() {
            instance_ = std::shared_ptr<T>(new T);
        });

        return instance_;
    }
    void PrintAddress()
    {
        cout<<instance_.get()<<endl;

    }
    ~Singleton(){
        cout<<"this is singleton destruct"<<endl;
    }
};

template<typename T>
std::shared_ptr<T> Singleton<T>::instance_ = nullptr;

#endif // SINGLETON_H
