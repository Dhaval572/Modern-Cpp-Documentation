// Easy and simple example of singleton
#include <iostream>
using namespace std;

class Singleton
{
public:
    static Singleton &Get()
    {
        static Singleton instance;
        return instance;
    }

    float getData(){ return Get().dataMember; }
    void setData(float value) { dataMember = value; }
    void Function() { cout << "I'm function of Singleton class\n"; }

private:
    Singleton() { cout << "I'm Constructor of Singleton class!\n"; }
    Singleton(const Singleton &) = delete;            // Delete the copy constructor
    Singleton &operator=(const Singleton &) = delete; // Delete assignment

    float dataMember = 0.0f;
};

int main()
{
    // First call to Get() creates the Singleton instance (constructor runs once)
    Singleton &obj = Singleton::Get();
    obj.Function();
    cout << "Default data of the obj is: " << obj.getData() << endl;
    obj.setData(42.0f);
    cout << "Setted data of the obj is: " << obj.getData() << endl;

    printf("\n\nObject 2 data\n");

    Singleton &obj2 = Singleton::Get();
    obj2.Function();
    obj2.setData(55.345f);
    cout << "Setted data of the obj2 is: " << obj2.getData() << endl;

    // obj2 = obj; // Will return error cause of no copy assginment allows ( Line no. 20 )
    // Singleton obj3 = Singleton::Get(); // Will also give error because not copy allows ( Line no. 19 )

    // Note: Here, You're not creating multiple instances, you're just accessing the same one multiple times.

    return 0;
}
