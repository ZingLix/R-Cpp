# R-Cpp

## Example

```
import std::io;

namespace rcpp{

export fn triple(Complex c){
    return Complex(c.a()*2, c.b()*2);
}

trait Sortable<T>{
    fn operator<(const T&) -> bool;
}

fn have_fun(){
    static i32 i;
    return 1, ref(i), Complex(2, 4);
}

fn sort(Iterable & Span arr){
    sort(arr.begin(), arr.end());
}

fn sort<T=Iterable<Sortable>>(T beg, T end) {
    //...
}

[[Sortable]]
class Complex{
public:
    constructor():a_(0),b_(0)
    {}
    
    constructor(i32 a, i32 b):a_(a), b_(b)
    {}

    fn operator<(const Complex& rhs){
        if(a_==rhs.a_){
            return b_<rhs.b_;
        }
        return a_<rhs.a_;
    }

    fn a(){ return a_; }
    fn b(){ return b_; }

private:
    i32 a_, b_;
}

}

fn main(){
    Vec<Complex> vec;
    vec.emplace_back(3, 2);
    // ...
    for &i in vec{
        if(i.a() % 2==0) triple(i);
    }
    ptr p = to(vec);
    u_ptr up = Complex::new(1,2);
    r_ptr<Sortable> rp = Complex::new(3,4);
    rp.release(); 
    return 0;
}
```