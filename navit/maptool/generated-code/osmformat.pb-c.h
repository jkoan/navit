/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: osmformat.proto */

#ifndef PROTOBUF_C_osmformat_2eproto__INCLUDED
#define PROTOBUF_C_osmformat_2eproto__INCLUDED

#include <google/protobuf-c/protobuf-c.h>

PROTOBUF_C__BEGIN_DECLS

#if PROTOBUF_C_VERSION_NUMBER < 1000000
# error This file was generated by a newer version of protoc-c which is incompatible with your libprotobuf-c headers. Please update your headers.
#elif 1003000 < PROTOBUF_C_MIN_COMPILER_VERSION
# error This file was generated by an older version of protoc-c which is incompatible with your libprotobuf-c headers. Please regenerate this file with a newer version of protoc-c.
#endif


typedef struct _OSMPBF__HeaderBlock OSMPBF__HeaderBlock;
typedef struct _OSMPBF__HeaderBBox OSMPBF__HeaderBBox;
typedef struct _OSMPBF__PrimitiveBlock OSMPBF__PrimitiveBlock;
typedef struct _OSMPBF__PrimitiveGroup OSMPBF__PrimitiveGroup;
typedef struct _OSMPBF__StringTable OSMPBF__StringTable;
typedef struct _OSMPBF__Info OSMPBF__Info;
typedef struct _OSMPBF__DenseInfo OSMPBF__DenseInfo;
typedef struct _OSMPBF__ChangeSet OSMPBF__ChangeSet;
typedef struct _OSMPBF__Node OSMPBF__Node;
typedef struct _OSMPBF__DenseNodes OSMPBF__DenseNodes;
typedef struct _OSMPBF__Way OSMPBF__Way;
typedef struct _OSMPBF__Relation OSMPBF__Relation;


/* --- enums --- */

typedef enum _OSMPBF__Relation__MemberType {
  OSMPBF__RELATION__MEMBER_TYPE__NODE = 0,
  OSMPBF__RELATION__MEMBER_TYPE__WAY = 1,
  OSMPBF__RELATION__MEMBER_TYPE__RELATION = 2
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(OSMPBF__RELATION__MEMBER_TYPE)
} OSMPBF__Relation__MemberType;

/* --- messages --- */

struct  _OSMPBF__HeaderBlock
{
  ProtobufCMessage base;
  OSMPBF__HeaderBBox *bbox;
  /*
   * Additional tags to aid in parsing this dataset
   */
  size_t n_required_features;
  char **required_features;
  size_t n_optional_features;
  char **optional_features;
  char *writingprogram;
  /*
   * From the bbox field.
   */
  char *source;
  /*
   * replication timestamp, expressed in seconds since the epoch,
   * otherwise the same value as in the "timestamp=..." field
   * in the state.txt file used by Osmosis
   */
  protobuf_c_boolean has_osmosis_replication_timestamp;
  int64_t osmosis_replication_timestamp;
  /*
   * replication sequence number (sequenceNumber in state.txt)
   */
  protobuf_c_boolean has_osmosis_replication_sequence_number;
  int64_t osmosis_replication_sequence_number;
  /*
   * replication base URL (from Osmosis' configuration.txt file)
   */
  char *osmosis_replication_base_url;
};
#define OSMPBF__HEADER_BLOCK__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&osmpbf__header_block__descriptor) \
    , NULL, 0,NULL, 0,NULL, NULL, NULL, 0, 0, 0, 0, NULL }


struct  _OSMPBF__HeaderBBox
{
  ProtobufCMessage base;
  int64_t left;
  int64_t right;
  int64_t top;
  int64_t bottom;
};
#define OSMPBF__HEADER_BBOX__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&osmpbf__header_bbox__descriptor) \
    , 0, 0, 0, 0 }


struct  _OSMPBF__PrimitiveBlock
{
  ProtobufCMessage base;
  OSMPBF__StringTable *stringtable;
  size_t n_primitivegroup;
  OSMPBF__PrimitiveGroup **primitivegroup;
  /*
   * Granularity, units of nanodegrees, used to store coordinates in this block
   */
  protobuf_c_boolean has_granularity;
  int32_t granularity;
  /*
   * Offset value between the output coordinates coordinates and the granularity grid in unites of nanodegrees.
   */
  protobuf_c_boolean has_lat_offset;
  int64_t lat_offset;
  protobuf_c_boolean has_lon_offset;
  int64_t lon_offset;
  /*
   * Granularity of dates, normally represented in units of milliseconds since the 1970 epoch.
   */
  protobuf_c_boolean has_date_granularity;
  int32_t date_granularity;
};
#define OSMPBF__PRIMITIVE_BLOCK__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&osmpbf__primitive_block__descriptor) \
    , NULL, 0,NULL, 0, 100, 0, 0ll, 0, 0ll, 0, 1000 }


/*
 * Group of OSMPrimitives. All primitives in a group must be the same type.
 */
struct  _OSMPBF__PrimitiveGroup
{
  ProtobufCMessage base;
  size_t n_nodes;
  OSMPBF__Node **nodes;
  OSMPBF__DenseNodes *dense;
  size_t n_ways;
  OSMPBF__Way **ways;
  size_t n_relations;
  OSMPBF__Relation **relations;
  size_t n_changesets;
  OSMPBF__ChangeSet **changesets;
};
#define OSMPBF__PRIMITIVE_GROUP__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&osmpbf__primitive_group__descriptor) \
    , 0,NULL, NULL, 0,NULL, 0,NULL, 0,NULL }


/*
 ** String table, contains the common strings in each block.
 *Note that we reserve index '0' as a delimiter, so the entry at that
 *index in the table is ALWAYS blank and unused.
 */
struct  _OSMPBF__StringTable
{
  ProtobufCMessage base;
  size_t n_s;
  ProtobufCBinaryData *s;
};
#define OSMPBF__STRING_TABLE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&osmpbf__string_table__descriptor) \
    , 0,NULL }


/*
 * Optional metadata that may be included into each primitive.
 */
struct  _OSMPBF__Info
{
  ProtobufCMessage base;
  protobuf_c_boolean has_version;
  int32_t version;
  protobuf_c_boolean has_timestamp;
  int64_t timestamp;
  protobuf_c_boolean has_changeset;
  int64_t changeset;
  protobuf_c_boolean has_uid;
  int32_t uid;
  /*
   * String IDs
   */
  protobuf_c_boolean has_user_sid;
  uint32_t user_sid;
  /*
   * The visible flag is used to store history information. It indicates that
   * the current object version has been created by a delete operation on the
   * OSM API.
   * When a writer sets this flag, it MUST add a required_features tag with
   * value "HistoricalInformation" to the HeaderBlock.
   * If this flag is not available for some object it MUST be assumed to be
   * true if the file has the required_features tag "HistoricalInformation"
   * set.
   */
  protobuf_c_boolean has_visible;
  protobuf_c_boolean visible;
};
#define OSMPBF__INFO__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&osmpbf__info__descriptor) \
    , 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }


/*
 ** Optional metadata that may be included into each primitive. Special dense format used in DenseNodes.
 */
struct  _OSMPBF__DenseInfo
{
  ProtobufCMessage base;
  size_t n_version;
  int32_t *version;
  /*
   * DELTA coded
   */
  size_t n_timestamp;
  int64_t *timestamp;
  /*
   * DELTA coded
   */
  size_t n_changeset;
  int64_t *changeset;
  /*
   * DELTA coded
   */
  size_t n_uid;
  int32_t *uid;
  /*
   * String IDs for usernames. DELTA coded
   */
  size_t n_user_sid;
  int32_t *user_sid;
  /*
   * The visible flag is used to store history information. It indicates that
   * the current object version has been created by a delete operation on the
   * OSM API.
   * When a writer sets this flag, it MUST add a required_features tag with
   * value "HistoricalInformation" to the HeaderBlock.
   * If this flag is not available for some object it MUST be assumed to be
   * true if the file has the required_features tag "HistoricalInformation"
   * set.
   */
  size_t n_visible;
  protobuf_c_boolean *visible;
};
#define OSMPBF__DENSE_INFO__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&osmpbf__dense_info__descriptor) \
    , 0,NULL, 0,NULL, 0,NULL, 0,NULL, 0,NULL, 0,NULL }


/*
 * THIS IS STUB DESIGN FOR CHANGESETS. NOT USED RIGHT NOW.
 * TODO:    REMOVE THIS?
 */
struct  _OSMPBF__ChangeSet
{
  ProtobufCMessage base;
  /*
   *
   *   // Parallel arrays.
   *   repeated uint32 keys = 2 [packed = true]; // String IDs.
   *   repeated uint32 vals = 3 [packed = true]; // String IDs.
   *   optional Info info = 4;
   */
  int64_t id;
};
#define OSMPBF__CHANGE_SET__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&osmpbf__change_set__descriptor) \
    , 0 }


struct  _OSMPBF__Node
{
  ProtobufCMessage base;
  int64_t id;
  /*
   * Parallel arrays.
   */
  /*
   * String IDs.
   */
  size_t n_keys;
  uint32_t *keys;
  /*
   * String IDs.
   */
  size_t n_vals;
  uint32_t *vals;
  /*
   * May be omitted in omitmeta
   */
  OSMPBF__Info *info;
  int64_t lat;
  int64_t lon;
};
#define OSMPBF__NODE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&osmpbf__node__descriptor) \
    , 0, 0,NULL, 0,NULL, NULL, 0, 0 }


struct  _OSMPBF__DenseNodes
{
  ProtobufCMessage base;
  /*
   * DELTA coded
   */
  size_t n_id;
  int64_t *id;
  /*
   *repeated Info info = 4;
   */
  OSMPBF__DenseInfo *denseinfo;
  /*
   * DELTA coded
   */
  size_t n_lat;
  int64_t *lat;
  /*
   * DELTA coded
   */
  size_t n_lon;
  int64_t *lon;
  /*
   * Special packing of keys and vals into one array. May be empty if all nodes in this block are tagless.
   */
  size_t n_keys_vals;
  int32_t *keys_vals;
};
#define OSMPBF__DENSE_NODES__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&osmpbf__dense_nodes__descriptor) \
    , 0,NULL, NULL, 0,NULL, 0,NULL, 0,NULL }


struct  _OSMPBF__Way
{
  ProtobufCMessage base;
  int64_t id;
  /*
   * Parallel arrays.
   */
  size_t n_keys;
  uint32_t *keys;
  size_t n_vals;
  uint32_t *vals;
  OSMPBF__Info *info;
  /*
   * DELTA coded
   */
  size_t n_refs;
  int64_t *refs;
};
#define OSMPBF__WAY__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&osmpbf__way__descriptor) \
    , 0, 0,NULL, 0,NULL, NULL, 0,NULL }


struct  _OSMPBF__Relation
{
  ProtobufCMessage base;
  int64_t id;
  /*
   * Parallel arrays.
   */
  size_t n_keys;
  uint32_t *keys;
  size_t n_vals;
  uint32_t *vals;
  OSMPBF__Info *info;
  /*
   * Parallel arrays
   */
  /*
   * This should have been defined as uint32 for consistency, but it is now too late to change it
   */
  size_t n_roles_sid;
  int32_t *roles_sid;
  /*
   * DELTA encoded
   */
  size_t n_memids;
  int64_t *memids;
  size_t n_types;
  OSMPBF__Relation__MemberType *types;
};
#define OSMPBF__RELATION__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&osmpbf__relation__descriptor) \
    , 0, 0,NULL, 0,NULL, NULL, 0,NULL, 0,NULL, 0,NULL }


/* OSMPBF__HeaderBlock methods */
void   osmpbf__header_block__init
                     (OSMPBF__HeaderBlock         *message);
size_t osmpbf__header_block__get_packed_size
                     (const OSMPBF__HeaderBlock   *message);
size_t osmpbf__header_block__pack
                     (const OSMPBF__HeaderBlock   *message,
                      uint8_t             *out);
size_t osmpbf__header_block__pack_to_buffer
                     (const OSMPBF__HeaderBlock   *message,
                      ProtobufCBuffer     *buffer);
OSMPBF__HeaderBlock *
       osmpbf__header_block__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   osmpbf__header_block__free_unpacked
                     (OSMPBF__HeaderBlock *message,
                      ProtobufCAllocator *allocator);
/* OSMPBF__HeaderBBox methods */
void   osmpbf__header_bbox__init
                     (OSMPBF__HeaderBBox         *message);
size_t osmpbf__header_bbox__get_packed_size
                     (const OSMPBF__HeaderBBox   *message);
size_t osmpbf__header_bbox__pack
                     (const OSMPBF__HeaderBBox   *message,
                      uint8_t             *out);
size_t osmpbf__header_bbox__pack_to_buffer
                     (const OSMPBF__HeaderBBox   *message,
                      ProtobufCBuffer     *buffer);
OSMPBF__HeaderBBox *
       osmpbf__header_bbox__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   osmpbf__header_bbox__free_unpacked
                     (OSMPBF__HeaderBBox *message,
                      ProtobufCAllocator *allocator);
/* OSMPBF__PrimitiveBlock methods */
void   osmpbf__primitive_block__init
                     (OSMPBF__PrimitiveBlock         *message);
size_t osmpbf__primitive_block__get_packed_size
                     (const OSMPBF__PrimitiveBlock   *message);
size_t osmpbf__primitive_block__pack
                     (const OSMPBF__PrimitiveBlock   *message,
                      uint8_t             *out);
size_t osmpbf__primitive_block__pack_to_buffer
                     (const OSMPBF__PrimitiveBlock   *message,
                      ProtobufCBuffer     *buffer);
OSMPBF__PrimitiveBlock *
       osmpbf__primitive_block__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   osmpbf__primitive_block__free_unpacked
                     (OSMPBF__PrimitiveBlock *message,
                      ProtobufCAllocator *allocator);
/* OSMPBF__PrimitiveGroup methods */
void   osmpbf__primitive_group__init
                     (OSMPBF__PrimitiveGroup         *message);
size_t osmpbf__primitive_group__get_packed_size
                     (const OSMPBF__PrimitiveGroup   *message);
size_t osmpbf__primitive_group__pack
                     (const OSMPBF__PrimitiveGroup   *message,
                      uint8_t             *out);
size_t osmpbf__primitive_group__pack_to_buffer
                     (const OSMPBF__PrimitiveGroup   *message,
                      ProtobufCBuffer     *buffer);
OSMPBF__PrimitiveGroup *
       osmpbf__primitive_group__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   osmpbf__primitive_group__free_unpacked
                     (OSMPBF__PrimitiveGroup *message,
                      ProtobufCAllocator *allocator);
/* OSMPBF__StringTable methods */
void   osmpbf__string_table__init
                     (OSMPBF__StringTable         *message);
size_t osmpbf__string_table__get_packed_size
                     (const OSMPBF__StringTable   *message);
size_t osmpbf__string_table__pack
                     (const OSMPBF__StringTable   *message,
                      uint8_t             *out);
size_t osmpbf__string_table__pack_to_buffer
                     (const OSMPBF__StringTable   *message,
                      ProtobufCBuffer     *buffer);
OSMPBF__StringTable *
       osmpbf__string_table__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   osmpbf__string_table__free_unpacked
                     (OSMPBF__StringTable *message,
                      ProtobufCAllocator *allocator);
/* OSMPBF__Info methods */
void   osmpbf__info__init
                     (OSMPBF__Info         *message);
size_t osmpbf__info__get_packed_size
                     (const OSMPBF__Info   *message);
size_t osmpbf__info__pack
                     (const OSMPBF__Info   *message,
                      uint8_t             *out);
size_t osmpbf__info__pack_to_buffer
                     (const OSMPBF__Info   *message,
                      ProtobufCBuffer     *buffer);
OSMPBF__Info *
       osmpbf__info__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   osmpbf__info__free_unpacked
                     (OSMPBF__Info *message,
                      ProtobufCAllocator *allocator);
/* OSMPBF__DenseInfo methods */
void   osmpbf__dense_info__init
                     (OSMPBF__DenseInfo         *message);
size_t osmpbf__dense_info__get_packed_size
                     (const OSMPBF__DenseInfo   *message);
size_t osmpbf__dense_info__pack
                     (const OSMPBF__DenseInfo   *message,
                      uint8_t             *out);
size_t osmpbf__dense_info__pack_to_buffer
                     (const OSMPBF__DenseInfo   *message,
                      ProtobufCBuffer     *buffer);
OSMPBF__DenseInfo *
       osmpbf__dense_info__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   osmpbf__dense_info__free_unpacked
                     (OSMPBF__DenseInfo *message,
                      ProtobufCAllocator *allocator);
/* OSMPBF__ChangeSet methods */
void   osmpbf__change_set__init
                     (OSMPBF__ChangeSet         *message);
size_t osmpbf__change_set__get_packed_size
                     (const OSMPBF__ChangeSet   *message);
size_t osmpbf__change_set__pack
                     (const OSMPBF__ChangeSet   *message,
                      uint8_t             *out);
size_t osmpbf__change_set__pack_to_buffer
                     (const OSMPBF__ChangeSet   *message,
                      ProtobufCBuffer     *buffer);
OSMPBF__ChangeSet *
       osmpbf__change_set__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   osmpbf__change_set__free_unpacked
                     (OSMPBF__ChangeSet *message,
                      ProtobufCAllocator *allocator);
/* OSMPBF__Node methods */
void   osmpbf__node__init
                     (OSMPBF__Node         *message);
size_t osmpbf__node__get_packed_size
                     (const OSMPBF__Node   *message);
size_t osmpbf__node__pack
                     (const OSMPBF__Node   *message,
                      uint8_t             *out);
size_t osmpbf__node__pack_to_buffer
                     (const OSMPBF__Node   *message,
                      ProtobufCBuffer     *buffer);
OSMPBF__Node *
       osmpbf__node__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   osmpbf__node__free_unpacked
                     (OSMPBF__Node *message,
                      ProtobufCAllocator *allocator);
/* OSMPBF__DenseNodes methods */
void   osmpbf__dense_nodes__init
                     (OSMPBF__DenseNodes         *message);
size_t osmpbf__dense_nodes__get_packed_size
                     (const OSMPBF__DenseNodes   *message);
size_t osmpbf__dense_nodes__pack
                     (const OSMPBF__DenseNodes   *message,
                      uint8_t             *out);
size_t osmpbf__dense_nodes__pack_to_buffer
                     (const OSMPBF__DenseNodes   *message,
                      ProtobufCBuffer     *buffer);
OSMPBF__DenseNodes *
       osmpbf__dense_nodes__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   osmpbf__dense_nodes__free_unpacked
                     (OSMPBF__DenseNodes *message,
                      ProtobufCAllocator *allocator);
/* OSMPBF__Way methods */
void   osmpbf__way__init
                     (OSMPBF__Way         *message);
size_t osmpbf__way__get_packed_size
                     (const OSMPBF__Way   *message);
size_t osmpbf__way__pack
                     (const OSMPBF__Way   *message,
                      uint8_t             *out);
size_t osmpbf__way__pack_to_buffer
                     (const OSMPBF__Way   *message,
                      ProtobufCBuffer     *buffer);
OSMPBF__Way *
       osmpbf__way__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   osmpbf__way__free_unpacked
                     (OSMPBF__Way *message,
                      ProtobufCAllocator *allocator);
/* OSMPBF__Relation methods */
void   osmpbf__relation__init
                     (OSMPBF__Relation         *message);
size_t osmpbf__relation__get_packed_size
                     (const OSMPBF__Relation   *message);
size_t osmpbf__relation__pack
                     (const OSMPBF__Relation   *message,
                      uint8_t             *out);
size_t osmpbf__relation__pack_to_buffer
                     (const OSMPBF__Relation   *message,
                      ProtobufCBuffer     *buffer);
OSMPBF__Relation *
       osmpbf__relation__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   osmpbf__relation__free_unpacked
                     (OSMPBF__Relation *message,
                      ProtobufCAllocator *allocator);
/* --- per-message closures --- */

typedef void (*OSMPBF__HeaderBlock_Closure)
                 (const OSMPBF__HeaderBlock *message,
                  void *closure_data);
typedef void (*OSMPBF__HeaderBBox_Closure)
                 (const OSMPBF__HeaderBBox *message,
                  void *closure_data);
typedef void (*OSMPBF__PrimitiveBlock_Closure)
                 (const OSMPBF__PrimitiveBlock *message,
                  void *closure_data);
typedef void (*OSMPBF__PrimitiveGroup_Closure)
                 (const OSMPBF__PrimitiveGroup *message,
                  void *closure_data);
typedef void (*OSMPBF__StringTable_Closure)
                 (const OSMPBF__StringTable *message,
                  void *closure_data);
typedef void (*OSMPBF__Info_Closure)
                 (const OSMPBF__Info *message,
                  void *closure_data);
typedef void (*OSMPBF__DenseInfo_Closure)
                 (const OSMPBF__DenseInfo *message,
                  void *closure_data);
typedef void (*OSMPBF__ChangeSet_Closure)
                 (const OSMPBF__ChangeSet *message,
                  void *closure_data);
typedef void (*OSMPBF__Node_Closure)
                 (const OSMPBF__Node *message,
                  void *closure_data);
typedef void (*OSMPBF__DenseNodes_Closure)
                 (const OSMPBF__DenseNodes *message,
                  void *closure_data);
typedef void (*OSMPBF__Way_Closure)
                 (const OSMPBF__Way *message,
                  void *closure_data);
typedef void (*OSMPBF__Relation_Closure)
                 (const OSMPBF__Relation *message,
                  void *closure_data);

/* --- services --- */


/* --- descriptors --- */

extern const ProtobufCMessageDescriptor osmpbf__header_block__descriptor;
extern const ProtobufCMessageDescriptor osmpbf__header_bbox__descriptor;
extern const ProtobufCMessageDescriptor osmpbf__primitive_block__descriptor;
extern const ProtobufCMessageDescriptor osmpbf__primitive_group__descriptor;
extern const ProtobufCMessageDescriptor osmpbf__string_table__descriptor;
extern const ProtobufCMessageDescriptor osmpbf__info__descriptor;
extern const ProtobufCMessageDescriptor osmpbf__dense_info__descriptor;
extern const ProtobufCMessageDescriptor osmpbf__change_set__descriptor;
extern const ProtobufCMessageDescriptor osmpbf__node__descriptor;
extern const ProtobufCMessageDescriptor osmpbf__dense_nodes__descriptor;
extern const ProtobufCMessageDescriptor osmpbf__way__descriptor;
extern const ProtobufCMessageDescriptor osmpbf__relation__descriptor;
extern const ProtobufCEnumDescriptor    osmpbf__relation__member_type__descriptor;

PROTOBUF_C__END_DECLS


#endif  /* PROTOBUF_C_osmformat_2eproto__INCLUDED */
