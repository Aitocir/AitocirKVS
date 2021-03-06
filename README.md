# AitocirKVS
My free-time attempt at making an easy-to-use keyvalue store. Intended to be as easy to use as PlayerPrefs (Unity3d) or NSUserDefaults (iOS) for a local program with very basic storage needs, while being more portable.

# Under Alpha Development, works but not efficient or well tested!


# AitocirKVS design

AitocirKVS is inspired by the API for Unity3d's PlayerPrefs, and the on-disk format from [cdb](http://cr.yp.to/cdb.html). It consists of a single binary file capable of storing usigned ints, signed ints, floats/doubles, booleans, and length-limited ASCII strings. The binary file is structured as follows:

    1) 14 bytes of header
      1a) 4 bytes, hard-wired to "AKVS" in ASCII for file verification
      1b) 2 bytes, unsigned integer indicating fixed key length to be used
      1c) 8 bytes, data used to decide how to weight what hash buckets get various value lengths
    2) 65536*4 bytes of hash indexes
      2a) first 4 bytes is a file byte address to the first element of the 0th hash bucket
      2b) second 4 bytes is a file byte address to the first element of the 1st hash bucket
      2c) ...
    3) n-byte length records appended one right after another
      3a) key (m bytes | m = value from 1b)
      3b) type, indicating what kind of value follows (1 byte)
      3c) value (p bytes | p = length of type from 3b)
      3d) next, 4 byte pointer to the file byte address of the next record in this hash bucket
      
Note that this database, like cdb, is limited to a 4GB file size because it uses 4 byte unsigned integers as file byte address pointers. 

The commands AitocirKVS supports are as follows:

  SET
    This command will update the key if it already exists or add it if it doesn't. It uses the [Fowler-Noll-Vo](https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function) hash function to hash the key, then masks the top 4 bits with a value to ensure the hash goes into a hash bucket with the same value length as the value type. Each hash bucket only has values of a fixed length. For example, a single hash bucket containing 4 byte values may have uint32s and int32s in it, but no uint16s because it can only hold values of 4 bytes in length. Then it looks down that hash bucket for the end element. If it finds a deleted element along the way, it will re-use that space for this record and not append anything. Otherwise, it will append a new record and point that previous tail record to this new one.
    
  GET
    Gets value for the provided key and value type. (We're requiring a type so that eventaully the wrapper API knows what datatype it needs to return at call time. This isn't an issue for the delete command, which does not require the key's value type. See below.)
    
  DELETE
    Remove the key-value pair from the database by NULL-ing out the bytes from the record except for the next pointer. The reason for this is that the NULL-ed space can be re-used by the next ADD that uses this hash bucket instead of appending a new record to the end.
