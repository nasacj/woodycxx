/*
 * LRUCache.h
 *
 *  Created on: 2014-12-01
 *      Author: qianchj
 */

#ifndef WOODYCXX_COMM_LRUCACHE_H_
#define WOODYCXX_COMM_LRUCACHE_H_

#include <map>
#include <list>

using namespace std;

namespace woodycxx {

    template<typename KeyType, typename ValueType>
    struct CacheNode{
        KeyType key;
        ValueType value;
        CacheNode(KeyType k , ValueType v) : key(k) , value(v){}
    };

    template<typename KeyType, typename ValueType>
    class LRUCache
    {
    public:
        LRUCache(int capacity)
        {
            size = capacity;
        }

        ValueType get(KeyType key)
        {
            if(cacheMap.find(key) != cacheMap.end())
            {
                auto it = cacheMap[key];
                cacheList.splice(cacheList.begin() , cacheList , it);
                cacheMap[key] = cacheList.begin();
                return cacheList.begin()->value;
            }
            else
            {
                return -1;
            }
        }

        void set(KeyType key, ValueType value)
        {
            if (cacheMap.find(key) == cacheMap.end())
            {
                if(cacheList.size() == size)
                {
                    cacheMap.erase(cacheList.back().key);
                    cacheList.pop_back();
                }
                cacheList.push_front(CacheNode<KeyType, ValueType>(key , value));
                cacheMap[key] = cacheList.begin();
            }
            else
            {
                auto it = cacheMap[key];
                cacheList.splice(cacheList.begin() , cacheList , it);
                cacheMap[key] = cacheList.begin();
                cacheList.begin()->value = value;
            }
        }

    private:
        int size;
        list< CacheNode<KeyType, ValueType> > cacheList;
        map< KeyType, typename list< CacheNode<KeyType, ValueType> >::iterator > cacheMap;
    };

}//end of namespace woodycxx


#endif
