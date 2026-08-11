# Virtual Functions in C++ – The Complete Guide (C++11 to C++23)

This guide takes you from the *why* to the *how* of virtual functions, covers best practices, explains how they work under the hood, compares C++ with Rust, and shows you the **Qt way** of using virtual functions – with official documentation references and tips & tricks.

---

## Table of Contents

1. [The Problem – Why Do We Need Virtual Functions?](#1-the-problem--why-do-we-need-virtual-functions)
2. [The Solution – Virtual Functions](#2-the-solution--virtual-functions)
3. [How Virtual Functions Work Under the Hood](#3-how-virtual-functions-work-under-the-hood)
4. [Best Practices for Virtual Functions](#4-best-practices-for-virtual-functions)
5. [Pure Virtual Functions and Abstract Classes](#5-pure-virtual-functions-and-abstract-classes)
6. [Virtual Destructors – Why They Matter](#6-virtual-destructors--why-they-matter)
7. [When to Use Virtual Functions – and When Not To](#7-when-to-use-virtual-functions--and-when-not-to)
8. [Virtual Functions in Unreal Engine – What’s Different?](#8-virtual-functions-in-unreal-engine--whats-different)
9. [C++ vs Rust – Virtual Functions vs Trait Objects](#9-c-vs-rust--virtual-functions-vs-trait-objects)
10. [The Qt Way – Using Virtual Functions in Qt](#10-the-qt-way--using-virtual-functions-in-qt)
11. [Tips and Tricks for Virtual Functions](#11-tips-and-tricks-for-virtual-functions)
12. [Summary](#12-summary)

---

## 1. The Problem – Why Do We Need Virtual Functions?

Imagine you have a base class `Animal` and a derived class `Dog`. You want to store a `Dog` in a container and call a `Speak()` function that should output the dog's sound, not a generic animal sound.

**Without `virtual`** – function calls are resolved at compile‑time (static binding). The compiler looks at the static type of the reference (`Animal&`) and calls `Animal::Speak()`, even if the object is actually a `Dog`.

```cpp
#include <iostream>
#include <memory>
#include <vector>

class Animal
{
public:
    void Speak()   // not virtual
    {
        std::cout << "Some animal sound\n";
    }
    
    virtual ~Animal() = default;
};

class Dog : public Animal
{
public:
    void Speak()
    {
        std::cout << "Woof!\n";
    }
};

void MakeItSpeak(Animal& animal)
{
    animal.Speak();   // Always calls Animal::Speak()
}

int main()
{
    Dog d;
    MakeItSpeak(d);   // Outputs "Some animal sound" – not what we want!
}
```

We need a mechanism that decides *at runtime* which function to call, based on the actual type of the object. That mechanism is **dynamic method dispatch**, implemented via **virtual functions**.

---

## 2. The Solution – Virtual Functions

By marking `Speak()` as `virtual` in the base class, we enable **late binding**. The compiler generates code that looks up the correct function to call at runtime.

```cpp
#include <iostream>
#include <memory>
#include <vector>

class Animal
{
public:
    virtual void Speak()   // now virtual
    {
        std::cout << "Some animal sound\n";
    }
    
    virtual ~Animal() = default;
};

class Dog : public Animal
{
public:
    void Speak() override
    {
        std::cout << "Woof!\n";
    }
};

class Cat : public Animal
{
public:
    void Speak() override
    {
        std::cout << "Meow!\n";
    }
};

void MakeItSpeak(Animal& animal)
{
    animal.Speak();   // Calls Dog::Speak() if animal is a Dog
}

int main()
{
    std::vector<std::unique_ptr<Animal>> animals;
    animals.push_back(std::make_unique<Dog>());
    animals.push_back(std::make_unique<Cat>());
    
    for (const auto& animal_ptr : animals)
    {
        MakeItSpeak(*animal_ptr);   // Outputs "Woof!" then "Meow!"
    }
}
```

Now the call is resolved at runtime based on the *dynamic type* of the object.

---

## 3. How Virtual Functions Work Under the Hood

### The vtable and vptr

Every class that has (or inherits) virtual functions gets a **virtual table** (vtable) – a static array of function pointers. Each object of such a class contains a hidden pointer (the **vptr**) that points to the vtable of its actual class.

- When you call a virtual function, the compiler generates code that:
  1. Follows the object's `vptr` to the vtable.
  2. Indexes into the vtable to get the correct function pointer.
  3. Calls that function.

**Memory layout example**:

```
class Animal
{
    vptr -> Animal_vtable: [ &Animal::Speak, &Animal::~Animal ]
    // other members...
};

class Dog
{
    vptr -> Dog_vtable: [ &Dog::Speak, &Dog::~Dog ]
    // other members...
};
```

When you pass a `Dog` object through an `Animal&` reference, the `vptr` inside the object remains pointing to `Dog`'s vtable, so the call resolves correctly.

> **Important**: The vtable is set up during object construction and remains fixed for the lifetime of the object.

### Performance Cost

- **Memory**: Each object carries one extra pointer (the vptr) – typically 8 bytes on 64‑bit systems. Each class with virtual functions has one vtable.
- **Time**: A virtual call typically costs an extra indirection (two pointer dereferences) compared to a direct function call. In modern CPUs, this is negligible for most use cases, but in tight loops it can matter.

---

## 4. Best Practices for Virtual Functions

### ✅ Use `virtual` in the base class for functions you intend to override

If you don't mark the base function as `virtual`, you'll get *early binding* – overriding will be hidden and only works through the derived type directly.

### ✅ Always use `override` (C++11) in derived classes

```cpp
class Dog : public Animal
{
public:
    void Speak() override   // compiler verifies signature match
    {
        // ...
    }
};
```

- **Why**: It catches errors like misspelling or mismatched signatures. If the base function is not `virtual` or has a different signature, the compiler will flag it.

### ✅ Use `final` to stop further overriding (when needed)

```cpp
class Dog : public Animal
{
public:
    void Speak() override final
    {
        // ...
    }
};

class Puppy : public Dog
{
    // void Speak() override;   // ❌ ERROR – Speak() is final
};
```

Marking a class `final` enables the compiler to **devirtualize** calls when the static type is known, eliminating overhead.

### ✅ Make destructors `virtual` in polymorphic base classes

(See section 6 for details.)

---

## 5. Pure Virtual Functions and Abstract Classes

A **pure virtual function** is declared with `= 0`. It has no implementation in the base class and forces derived classes to provide one (unless they become abstract too).

```cpp
#include <iostream>
#include <memory>
#include <vector>
#include <string>

class Animal   // Abstract class – cannot be instantiated
{
public:
    virtual void Speak() = 0;   // pure virtual – must implement
    virtual std::string GetName() const = 0;
    
    // Non-pure virtual with default implementation
    virtual int GetLegs() const
    {
        return 4;
    }
    
    virtual ~Animal() = default;
};

class Dog : public Animal
{
private:
    std::string name_;
    
public:
    explicit Dog(std::string name)
        : name_(std::move(name))
    {
    }
    
    void Speak() override
    {
        std::cout << name_ << " says Woof!\n";
    }
    
    std::string GetName() const override
    {
        return name_;
    }
};

class Cat : public Animal
{
private:
    std::string name_;
    
public:
    explicit Cat(std::string name)
        : name_(std::move(name))
    {
    }
    
    void Speak() override
    {
        std::cout << name_ << " says Meow!\n";
    }
    
    std::string GetName() const override
    {
        return name_;
    }
};

void ProcessAnimal(Animal& animal)
{
    std::cout << animal.GetName() << " has " << animal.GetLegs() << " legs\n";
    animal.Speak();
}

int main()
{
    std::vector<std::unique_ptr<Animal>> animals;
    animals.push_back(std::make_unique<Dog>("Rex"));
    animals.push_back(std::make_unique<Cat>("Whiskers"));
    
    for (auto& animal_ptr : animals)
    {
        ProcessAnimal(*animal_ptr);
    }
}
```

You cannot instantiate an abstract class (`Animal a;` is illegal). Pure virtual functions are used to define an interface that derived classes must implement.

---

## 6. Virtual Destructors – Why They Matter

If you delete a derived object through a base pointer, and the base destructor is not virtual, only the base destructor runs – the derived part is never destroyed, causing resource leaks.

```cpp
class Base
{
public:
    ~Base()   // non-virtual
    {
        std::cout << "Base destructor\n";
    }
};

class Derived : public Base
{
public:
    ~Derived()
    {
        std::cout << "Derived destructor\n";
    }
};

int main()
{
    std::unique_ptr<Base> p = std::make_unique<Derived>();
    // Only "Base destructor" printed – Derived part leaks!
}
```

**Solution**: Make the base destructor `virtual`.

```cpp
class Base
{
public:
    virtual ~Base() = default;   // now the full destructor chain runs
};
```

**Rule**: If a class has any virtual function, it should have a virtual destructor. Even if it's not polymorphic, if you intend to derive from it, consider a virtual destructor.

---

## 7. When to Use Virtual Functions – and When Not To

### ✅ Use when:
- You need polymorphic behaviour (treating derived objects uniformly through base references).
- The exact type is not known until runtime.
- You are designing a framework or library where users will extend your classes.

### ❌ Avoid when:
- Performance is extremely critical (e.g., every nanosecond counts in a tight loop) – virtual calls have overhead.
- You do not need polymorphic deletion – but even then, if the class is intended to be derived, a virtual destructor is recommended.
- The class is not designed to be a base class (use `final` to prevent inheritance).

---

## 8. Virtual Functions in Unreal Engine – What’s Different?

Unreal Engine uses **standard C++ virtual functions** exactly as described above. However, Unreal adds its own reflection system via **UFUNCTION** macros for Blueprint exposure, serialization, etc. These macros are **not** related to C++ virtual dispatch; they are metadata for Unreal's build tool (UBT).

**Unreal’s coding standards**:
- Use `virtual` for base class functions that can be overridden.
- Use `override` in derived classes.
- **Virtual destructors** are strongly recommended for any class that may be used polymorphically – same as standard C++.

**What might be perceived as "different"**:
- Unreal often uses `BlueprintNativeEvent` – this is a macro that generates a virtual function with a default implementation in C++ that can be overridden in Blueprints.
- Unreal also uses `UFUNCTION` for reflection, which does not replace `virtual` – they serve different purposes.

**Example from Unreal Engine**:
```cpp
UCLASS()
class AMyCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override
    {
        Super::BeginPlay();   // Call base class
        // Custom initialization
    }

    UFUNCTION(BlueprintNativeEvent, Category = "Combat")
    void TakeDamage(float damage);
    virtual void TakeDamage_Implementation(float damage)
    {
        // Default C++ implementation
        health -= damage;
    }
};
```

So the advice is: **virtual functions in Unreal work exactly as in standard C++**. Follow the same best practices.

---

## 9. C++ vs Rust – Virtual Functions vs Trait Objects

Rust does not have inheritance; it uses **traits** for polymorphism. Dynamic dispatch in Rust is done via **trait objects** (e.g., `&dyn Animal` or `Box<dyn Animal>`). A trait object is a fat pointer containing a pointer to the data and a pointer to a vtable (similar to C++'s vptr).

### Similarities:
- Both use a vtable and a pointer indirection.
- Both have runtime overhead (two dereferences).
- Both support dynamic dispatch.

### Where C++ virtual functions (arguably) excel:

1. **Inheritance** – C++ allows you to mix dynamic dispatch with static dispatch on the same class, and you can have multiple levels of inheritance with shared vtables. Rust's trait objects are flatter; you can only have one trait object at a time.

2. **Performance tuning** – In C++, you can decide per function whether it should be `virtual` or not. In Rust, a method called on a trait object is always dynamically dispatched; if you want static dispatch, you must use generics (monomorphization).

3. **Multiple inheritance** – C++ supports multiple inheritance and virtual inheritance. Rust doesn't have multiple inheritance; you must use composition or trait delegation.

4. **Default implementations** – In C++, a virtual function can have a default implementation in the base class, and derived classes may or may not override it.

5. **Legacy code and ecosystem** – Many C++ libraries rely heavily on virtual inheritance and polymorphism.

> **Conclusion**: While Rust's trait objects are a clean and safe alternative, C++'s virtual functions offer a more mature, flexible, and performance-tunable model, especially for systems with deep inheritance hierarchies.

---

## 10. The Qt Way – Using Virtual Functions in Qt

Qt is a framework that makes extensive use of virtual functions to allow customization of its widgets, threads, and other components. The official Qt documentation provides many examples of overridable virtual functions.

### Common Virtual Functions in Qt

| Qt Class      | Typical Virtual Functions to Override |
|---------------|---------------------------------------|
| `QWidget`     | `paintEvent()`, `mousePressEvent()`, `keyPressEvent()`, `resizeEvent()`, `timerEvent()` |
| `QThread`     | `run()` – the entry point for the thread |
| `QObject`     | `event()` – handles events; `timerEvent()`; `childEvent()` |
| `QAbstractItemModel` | `data()`, `rowCount()`, `columnCount()`, `parent()`, `index()` |
| `QIODevice`   | `readData()`, `writeData()` – for custom I/O devices |

### Example: Overriding `paintEvent` in a Custom Widget

From the [Qt Widgets documentation](https://doc.qt.io/qt-6/qwidget.html#paintEvent):

```cpp
#include <QWidget>
#include <QPainter>

class MyWidget : public QWidget
{
public:
    MyWidget(QWidget* parent = nullptr)
        : QWidget(parent)
    {
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        QPainter painter(this);
        painter.fillRect(rect(), Qt::blue);
        painter.drawText(rect(), Qt::AlignCenter, "Hello, Qt!");
    }
};
```

**Important points from Qt documentation**:
- Always call the base class implementation if you need the default behaviour, unless you intend to replace it entirely.
- Use `override` to ensure you are correctly overriding a virtual function.
- The destructor of `QObject` is virtual, so it's safe to delete a derived widget through a `QObject*`.

### Example: Overriding `QThread::run()`

From the [QThread documentation](https://doc.qt.io/qt-6/qthread.html#run):

```cpp
#include <QThread>

class WorkerThread : public QThread
{
    void run() override
    {
        // Heavy computation here
        emit resultReady(compute());
    }

signals:
    void resultReady(int result);
};
```

**Best practice note**: In modern Qt, it's often recommended to use `QObject` and `moveToThread` rather than subclassing `QThread`, but the virtual `run()` remains a valid approach.

### Overriding `event()` for Custom Event Handling

The `event()` function in `QObject` is virtual and is the main entry point for all events. You can override it to intercept events before they are dispatched to specific handlers (like `mousePressEvent`).

From the [QObject::event() documentation](https://doc.qt.io/qt-6/qobject.html#event):

```cpp
#include <QObject>
#include <QEvent>
#include <QKeyEvent>

class MyObject : public QObject
{
protected:
    bool event(QEvent* e) override
    {
        if (e->type() == QEvent::KeyPress)
        {
            QKeyEvent* key = static_cast<QKeyEvent*>(e);
            if (key->key() == Qt::Key_Tab)
            {
                // Handle tab key specially
                return true;   // event accepted
            }
        }
        return QObject::event(e);   // forward other events
    }
};
```

### The `Q_OBJECT` Macro and Virtual Functions

The `Q_OBJECT` macro is required for classes that use signals/slots or the Qt meta-object system. It does **not** affect virtual function dispatch. However, it enables runtime type information (via `metaObject()`), which can be used to call virtual functions safely.

### Best Practices When Using Qt Virtual Functions

1. **Always use `override`** in derived classes to catch signature mismatches.
2. **Call the base class implementation** unless you have a specific reason not to – many Qt event handlers expect the base class to perform default processing.
3. **Do not call virtual functions from constructors or destructors** – the derived class vtable is not fully set up during construction/destruction.
4. **Make your own base classes have virtual destructors** – Qt does this by default for `QObject`, but for custom hierarchies, follow the same rule.
5. **Use `final` sparingly** – it can prevent further overriding, which may be desirable for optimization but limits extensibility.

### Reference to Official Qt Documentation

- [QWidget event handlers](https://doc.qt.io/qt-6/qwidget.html#events)
- [QThread::run()](https://doc.qt.io/qt-6/qthread.html#run)
- [QObject::event()](https://doc.qt.io/qt-6/qobject.html#event)
- [QAbstractItemModel](https://doc.qt.io/qt-6/qabstractitemmodel.html)
- [Qt's Object Model](https://doc.qt.io/qt-6/object.html)

---

## 11. Tips and Tricks for Virtual Functions

### Tip 1: Avoid Virtual Calls in Constructors and Destructors

During construction, the object's vptr points to the vtable of the class being constructed, not the final derived class. So calling a virtual function in the constructor will call the version of the current class, not a derived override. Similarly for destructors.

```cpp
class Base
{
public:
    Base()
    {
        Speak();   // Calls Base::Speak(), even if derived object
    }
    virtual void Speak()
    {
        std::cout << "Base\n";
    }
};

class Derived : public Base
{
public:
    void Speak() override
    {
        std::cout << "Derived\n";
    }
};

int main()
{
    Derived d;   // Outputs "Base" – not "Derived"
}
```

### Tip 2: Use `final` to Devirtualize Calls

If you mark a virtual function as `final` in a derived class, the compiler can devirtualize calls when the static type is known, potentially improving performance.

```cpp
class Derived final : public Base
{
public:
    void Speak() override final
    {
        // ...
    }
};

void Use(Derived& d)   // static type known
{
    d.Speak();   // May be inlined and devirtualized
}
```

### Tip 3: Consider `std::function` or Templates for Callbacks Instead of Virtual Functions

If you need polymorphic behaviour but don't require inheritance, consider using `std::function` or templates. Virtual functions are best when you have a fixed interface that many classes implement.

### Tip 4: Use `override` and `final` to Improve Readability and Catch Errors

These keywords are not just for the compiler – they also document your intent.

### Tip 5: Keep Interfaces Small (Interface Segregation Principle)

A large base class with many pure virtual functions forces every derived class to implement all of them. Prefer several small interfaces (use multiple inheritance if necessary).

### Tip 6: Understand the Cost – But Don't Over‑Optimize Prematurely

Virtual calls are cheap in most scenarios. Only worry about performance if profiling shows a hotspot. In GUI applications (like Qt), the overhead is negligible compared to rendering or event processing.

### Tip 7: Use CRTP (Curiously Recurring Template Pattern) for Static Polymorphism

If you need polymorphism but can know the derived type at compile time, consider CRTP. It avoids vtable overhead.

```cpp
template<typename Derived>
class Base
{
public:
    void Interface()
    {
        static_cast<Derived*>(this)->Implementation();
    }
};

class Derived : public Base<Derived>
{
public:
    void Implementation() { /* ... */ }
};
```

### Tip 8: In Qt, Prefer Signals and Slots Over Overriding Virtual Functions When Possible

Qt's signal/slot mechanism provides a more flexible, decoupled way to react to events. Overriding virtual event handlers is still necessary for custom drawing or low‑level event filtering.

### Tip 9: Use `Q_DECL_OVERRIDE` (Pre‑C++11) or `override` (C++11+)

Qt provides `Q_DECL_OVERRIDE` for compatibility, but in modern C++, just use `override`.

### Tip 10: Consider `Q_INVOKABLE` for Methods That Need to Be Called from QML

`Q_INVOKABLE` marks a method as callable from QML. It doesn't affect virtual dispatch but is often used alongside virtual functions in Qt.

---

## 12. Summary

| Key Point | Description |
|-----------|-------------|
| **Problem solved** | Virtual functions enable runtime polymorphism – calling the correct function based on the actual object type. |
| **Under the hood** | vptr + vtable – each object points to a table of function pointers. |
| **Best practices** | Use `virtual` in base, `override` in derived, `final` when needed, and always make destructors `virtual` in polymorphic base classes. |
| **Pure virtual** | Defines an interface; classes with pure virtual functions are abstract. |
| **When to use** | When runtime type matters and you need uniform treatment of derived objects. |
| **When to avoid** | When performance is ultra‑critical or when static polymorphism suffices. |
| **Unreal Engine** | Virtual functions are standard C++; `UFUNCTION` macros are separate metadata. |
| **Rust comparison** | C++ offers more flexible inheritance, finer control over static/dynamic dispatch, and a mature ecosystem. |
| **Qt usage** | Qt relies heavily on virtual functions for customisation (e.g., `paintEvent`, `run`, `event`). Always call base class implementations and use `override`. |
| **Tips** | Avoid virtual calls in ctors/dtors; use `final` for devirtualization; keep interfaces small; prefer signals/slots in Qt for decoupled event handling. |

---

## Final Example – Putting It All Together (with Qt)

Here is a complete, realistic Qt example following all best practices (naming: `snake_case` for variables, `PascalCase` for functions and classes, `t_PascalCase` for structs; braces on new lines).

```cpp
#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QKeyEvent>
#include <QDebug>
#include <memory>

class CustomWidget : public QWidget
{
public:
    explicit CustomWidget(QWidget* parent = nullptr)
        : QWidget(parent)
        , circle_color_(Qt::red)
    {
        setFocusPolicy(Qt::StrongFocus);
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        QPainter painter(this);
        painter.setBrush(circle_color_);
        painter.drawEllipse(rect().center(), 50, 50);
    }

    void keyPressEvent(QKeyEvent* event) override
    {
        switch (event->key())
        {
        case Qt::Key_Left:
            circle_color_ = Qt::blue;
            update();
            break;
        case Qt::Key_Right:
            circle_color_ = Qt::green;
            update();
            break;
        default:
            QWidget::keyPressEvent(event);
            break;
        }
    }

    bool event(QEvent* e) override
    {
        qDebug() << "Event type:" << e->type();
        return QWidget::event(e);
    }

private:
    QColor circle_color_;
};

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    auto widget = std::make_unique<CustomWidget>();
    widget->resize(300, 300);
    widget->show();

    return app.exec();
}
```

This demonstrates:
- `override` on all virtual functions.
- Calling the base class implementation where appropriate (`QWidget::keyPressEvent`, `QWidget::event`).
- A virtual destructor is inherited from `QWidget` (which itself has a virtual destructor), so we don't need to declare one.
- Naming conventions: `circle_color_` (snake_case), `CustomWidget` (PascalCase).

---

With this guide, you now have a solid understanding of virtual functions, their internals, best practices, and how to apply them in real‑world frameworks like Qt. Happy coding!
