[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/36XIBlW7)
## დავალების ატვირთვა
დავალება უნდა ატვირთოთ თქვენს პერსონალურ Github Classroom-ის რეპოზიტორიაში.

## საჭირო პაკეტები
დაგჭირდებათ libcurl პაკეტის დაყენება
```sh
dpkg --add-architecture i386
apt update
apt install gcc make
apt install libcurl4-openssl-dev:i386
apt install libnsl-dev:i386
apt install gcc-multilib
```

## კომპილაცია
```sh
make
```

## ტესტებისთვის საჭირო data ფაილები
`make` ის პირველი გაშვება ავტომატურად შექმნის data დირექტორიას ტესტებისთვის საჭირო ფაილებით.  
თუ რატომღაც ეს ფაილები "დაგიზიანდათ", მათი თავიდან ჩამოტვირთვისთვის გაუშვით:
```sh
rm -rf data/
make data
```

## ტესტირება
```sh
./assn-4-checker-64 ./rss-news-search
./assn-4-checker-64 ./rss-news-search -m
```

## ჰეშირების ფუნქცია
For those of you in need of a hash function for strings,
you can use the following, which is lifted from a textbook
we used to use in CS106A.  You'll need to modify it so that
it can be used by a hashset to store C strings (or structs
keyed on C strings).

```cpp
/** 
 * StringHash                     
 * ----------  
 * This function adapted from Eric Roberts' "The Art and Science of C"
 * It takes a string and uses it to derive a hash code, which   
 * is an integer in the range [0, numBuckets).  The hash code is computed  
 * using a method called "linear congruence."  A similar function using this     
 * method is described on page 144 of Kernighan and Ritchie.  The choice of                                                     
 * the value for the kHashMultiplier can have a significant effect on the                            
 * performance of the algorithm, but not on its correctness.                                                    
 * This hash function has the additional feature of being case-insensitive,  
 * hashing "Peter Pawlowski" and "PETER PAWLOWSKI" to the same code.  
 */  

static const signed long kHashMultiplier = -1664117991L;
static int StringHash(const char *s, int numBuckets)  
{            
  int i;
  unsigned long hashcode = 0;
  
  for (i = 0; i < strlen(s); i++)  
    hashcode = hashcode * kHashMultiplier + tolower(s[i]);  
  
  return hashcode % numBuckets;                                
}
```
