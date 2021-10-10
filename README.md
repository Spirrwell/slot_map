# slot_map

This is a very simple header only slot_map implementation in C++17. It's not full of a bunch of template nonsense. It's not the most blazing fast. But it keeps data contiguous, and iteration times should be the same as std::vector. It's pretty simple to use.

## Basic Usage

```cpp
spl::slot_map<std::string> sm;

// Access through a light-weight key
spl::slot_handle key = sm.emplace_back("Hello!");
std::string& str = sm[key];

// Access through a less safe wrapper
spl::slot_wrap<std::string> wrap = sm.as_wrap(key);

// You can de-reference or use the arrow operator
(*wrap).c_str();
wrap->c_str();
```
