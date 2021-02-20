#pragma once

template<typename T, u32 BucketSize>
struct Bucket {
    Bucket* next;
    Bucket* prev;
    u32 at;
    T data[BucketSize];
};

template<typename T, u32 BucketSize>
struct BucketArray {
    Allocator allocator;
    Bucket<T, BucketSize>* firstBucket;
    Bucket<T, BucketSize>* lastBucket;
    u32 bucketCount;
};

template<typename T, u32 BucketSize, typename Fn>
void _ForEach(BucketArray<T, BucketSize>* array, Fn fn) {
    auto bucket = array->firstBucket;
    do {
        for (u32 i = 0; i < bucket->at; i++) {
            T* it = bucket->data + i;
            fn(it);
        }
        bucket = bucket->next;
    } while(bucket != array->firstBucket);
}

template<typename T, u32 BucketSize>
Bucket<T, BucketSize>* BucketArrayGetBucket(BucketArray<T, BucketSize>* array) {
    auto bucket = (Bucket<T, BucketSize>*)array->allocator.Alloc(sizeof(Bucket<T, BucketSize>), false, alignof(Bucket<T, BucketSize>));
    if (bucket) {
        bucket->next = bucket;
        bucket->prev = bucket;
        bucket->at = 0;
    }
    return bucket;
}

template<typename T, u32 BucketSize>
bool BucketArrayInit(BucketArray<T, BucketSize>* array, Allocator allocator) {
    bool result = false;
    array->allocator = allocator;
    auto bucket = BucketArrayGetBucket(array);
    if (bucket) {
        array->firstBucket = bucket;
        array->lastBucket = bucket;
        array->bucketCount = 1;
        result = true;
    }
    return result;
}

template<typename T, u32 BucketSize>
struct BucketArrayEntry {
    Bucket<T, BucketSize>* bucket;
    T* ptr;
};

template<typename T, u32 BucketSize>
BucketArrayEntry<T, BucketSize> BucketArrayAdd(BucketArray<T, BucketSize>* array) {
    BucketArrayEntry<T, BucketSize> result = {};
    Bucket<T, BucketSize>* bucket;
    if (array->firstBucket->at == BucketSize) {
        bucket = BucketArrayGetBucket(array);
        if (bucket) {
            bucket->next = array->firstBucket;
            bucket->prev = array->lastBucket;
            array->firstBucket->prev = bucket;
            array->lastBucket->next = bucket;
            array->firstBucket = bucket;
        }
    } else {
        bucket = array->firstBucket;
    }

    if (bucket) {
        result = { bucket, bucket->data + bucket->at++ };
        if (bucket->at == BucketSize) {
            array->firstBucket = bucket->next;
            array->lastBucket = bucket;
        }
    }

    return result;
}

template<typename T, u32 BucketSize>
void BucketArrayRemove(BucketArray<T, BucketSize>* array, BucketArrayEntry<T, BucketSize> entry) {

}
