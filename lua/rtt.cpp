/*
 * Lua-RTT bindings.
 */

#include "rtt.hpp"

#include <rtt/TaskContext.hpp>
#include <rtt/Port.hpp>
#include <rtt/types/Types.hpp>
#include <rtt/base/DataSourceBase.hpp>
#include <rtt/types/Operators.hpp>
#include <rtt/Logger.hpp>

using namespace std;
using namespace RTT;
using namespace RTT::detail;
using namespace RTT::base;
using namespace RTT::internal;


/***************************************************************
 * Tricks from here: http://lua-users.org/wiki/DoItYourselfCppBinding
 ***************************************************************/

/* overloading new */
void* operator new(size_t size, lua_State* L, const char* metatableName)
{
	void* ptr = lua_newuserdata(L, size);
	luaL_getmetatable(L, metatableName);
	/* assert(lua_istable(L, -1)) */  /* if you're paranoid */
	lua_setmetatable(L, -2);
	return ptr;
}
/*
 * lua_pushobject(L, MyObj)("ctr_arg)
 * expands to
 * new(L, "MyObj") MyObj("ctr_arg")
 */
#define lua_pushobject(L, T) new(L, #T) T

/*
 * cast objects back
 */
#define lua_userdata_cast(L, pos, T) reinterpret_cast<T*>(luaL_checkudata((L), (pos), #T))

/*
 * get boxed (returns ptr**)
 */
#define lua_getudata_bx(L, pos, T) (T**) (luaL_checkudata((L), (pos), #T))

/*
 * variants which allows define metatable name independent of object
 * created.  used to create a base class O of T
 */
#define lua_pushobject2(L, MT, T) new(L, #MT) T
#define lua_userdata_cast2(L, pos, MT, T) reinterpret_cast<T*>(luaL_checkudata((L), (pos), #MT))
#define lua_getudata_bx2(L, pos, MT, T) (T**) (luaL_checkudata((L), (pos), #MT))

/* template for generating GC function */
template<typename T>
int GCMethod(lua_State* L)
{
	reinterpret_cast<T*>(lua_touserdata(L, 1))->~T();
	return 0;
}


/***************************************************************
 * Some generic helpers
 ***************************************************************/

/* test if userdata on position ud has metatable tname */
void* luaL_testudata (lua_State *L, int ud, const char *tname) {
	void *p = lua_touserdata(L, ud);
	if (p != NULL) {  /* value is a userdata? */
		if (lua_getmetatable(L, ud)) {  /* does it have a metatable? */
			lua_getfield(L, LUA_REGISTRYINDEX, tname);  /* get correct metatable */
			if (lua_rawequal(L, -1, -2)) {  /* does it have the correct mt? */
				lua_pop(L, 2);  /* remove both metatables */
				return p;
			}
		}
	}
	return NULL;
}


void push_vect_str(lua_State *L, const std::vector<std::string> &v)
{
	int key = 1;
	lua_createtable(L, v.size(), 0);

	for(vector<std::string>::const_iterator it = v.begin(); it != v.end(); ++it) {
		lua_pushstring(L, it->c_str());
		lua_rawseti(L, -2, key++);
	}
}

/***************************************************************
 * Property (boxed)
 ***************************************************************/
static int Property_new(lua_State *L)
{
	const char *type, *name, *desc;
	PropertyBase **pb;
	type = luaL_checkstring(L, 1);
	name = luaL_checkstring(L, 2);

	/* make description optional */
	if(lua_gettop(L) == 3)
		desc = luaL_checkstring(L, 3);
	else
		desc = "";

	types::TypeInfo *ti = types::TypeInfoRepository::Instance()->type(type);

	if(!ti)
		luaL_error(L, "Property.new: unknown type %s", type);

	pb = (PropertyBase**) lua_newuserdata(L, sizeof(PropertyBase*));
	*pb = ti->buildProperty(name, desc);
	luaL_getmetatable(L, "Property");
	lua_setmetatable(L, -2);
	return 1;
}

static int Property_get(lua_State *L)
{
	PropertyBase *pb = *(lua_getudata_bx2(L, 1, Property, PropertyBase));
	lua_pushobject2(L, Variable, DataSourceBase::shared_ptr)(pb->getDataSource());
	return 1;
}

static int Property_set(lua_State *L)
{
	PropertyBase *pb = *(lua_getudata_bx2(L, 1, Property, PropertyBase));
	DataSourceBase::shared_ptr newdsb = *(lua_userdata_cast2(L, 2, Variable, DataSourceBase::shared_ptr));
	DataSourceBase::shared_ptr propdsb = pb->getDataSource();
	propdsb->update(newdsb.get());
	return 1;
}

static int Property_getName(lua_State *L)
{
	PropertyBase *pb = *(lua_getudata_bx2(L, 1, Property, PropertyBase));
	lua_pushstring(L, pb->getName().c_str());
	return 1;
}

static int Property_getDescription(lua_State *L)
{
	PropertyBase *pb = *(lua_getudata_bx2(L, 1, Property, PropertyBase));
	lua_pushstring(L, pb->getDescription().c_str());
	return 1;
}


/*
 * Race condition if we collect properties: if we add this property to
 * a TC and our life ends before that of the TC, the property will be
 * deleted before the TaskContext.
 */
static int Property_gc(lua_State *L)
{
	PropertyBase *pb = *(lua_getudata_bx2(L, 1, Property, PropertyBase));
	delete pb;
	return 0;
}

static const struct luaL_Reg Property_f [] = {
	{"new", Property_new },
	{"get", Property_get },
	{"set", Property_set },
	{NULL, NULL}
};

static const struct luaL_Reg Property_m [] = {
	{"get", Property_get },
	{"set", Property_set },
	{"getName", Property_getName },
	{"getDescription", Property_getDescription },
	// todo: shall we or not? s.o. {"__gc", Property_gc },
	{NULL, NULL}
};

/***************************************************************
 * Ports (boxed)
 ***************************************************************/

/* InputPort (boxed) */
static int InputPort_new(lua_State *L)
{
	const char *type, *name;
	InputPortInterface** ip;
	type = luaL_checkstring(L, 1);
	name = luaL_checkstring(L, 2);
	types::TypeInfo *ti = types::TypeInfoRepository::Instance()->type(type);
	if(ti==0)
		luaL_error(L, "InputPort.new: unknown type %s", type);

	ip = (InputPortInterface**) lua_newuserdata(L, sizeof(InputPortInterface*));
	*ip = ti->inputPort(name);
	luaL_getmetatable(L, "InputPort");
	lua_setmetatable(L, -2);
	return 1;
}

static int InputPort_read(lua_State *L)
{
	InputPortInterface *ip = *(lua_getudata_bx2(L, 1, InputPort, InputPortInterface));
	DataSourceBase::shared_ptr *dsbp = lua_userdata_cast2(L, 2, Variable, DataSourceBase::shared_ptr);

	enum FlowStatus fs = ip->read(*dsbp);

	if(fs == NoData) lua_pushstring(L, "NoData");
	else if (fs == NewData) lua_pushstring(L, "NewData");
	else if (fs == OldData) lua_pushstring(L, "OldData");
	else luaL_error(L, "InputPort.read: unknown FlowStatus returned");
	return 1;
}

static int InputPort_gc(lua_State *L)
{
	InputPortInterface *ip = *(lua_getudata_bx2(L, 1, InputPort, InputPortInterface));
	delete ip;
 	return 0;
}

static const struct luaL_Reg InputPort_f [] = {
	{"new", InputPort_new },
	{"read", InputPort_read },
	{NULL, NULL}
};

static const struct luaL_Reg InputPort_m [] = {
	{"read", InputPort_read },
	{"__gc", InputPort_gc },
	{NULL, NULL}
};

/* OutputPort */
static int OutputPort_new(lua_State *L)
{
	const char *type, *name;
	OutputPortInterface** op;
	type = luaL_checkstring(L, 1);
	name = luaL_checkstring(L, 2);
	types::TypeInfo *ti = types::TypeInfoRepository::Instance()->type(type);
	if(ti==0)
		luaL_error(L, "OutputPort.new: unknown type %s", type);

	op = (OutputPortInterface**) lua_newuserdata(L, sizeof(OutputPortInterface*));
	*op = ti->outputPort(name);
	luaL_getmetatable(L, "OutputPort");
	lua_setmetatable(L, -2);
	return 1;
}

static int OutputPort_write(lua_State *L)
{
	OutputPortInterface *op = *(lua_getudata_bx2(L, 1, OutputPort, OutputPortInterface));
	DataSourceBase::shared_ptr *dsbp = lua_userdata_cast2(L, 2, Variable, DataSourceBase::shared_ptr);
	op->write(*dsbp);
	return 0;
}

static int OutputPort_gc(lua_State *L)
{
	OutputPortInterface *op = *(lua_getudata_bx2(L, 1, OutputPort, OutputPortInterface));
	delete op;
 	return 0;
}

static const struct luaL_Reg OutputPort_f [] = {
	{"new", OutputPort_new },
	{"write", OutputPort_write },
	{NULL, NULL}
};

static const struct luaL_Reg OutputPort_m [] = {
	{"write", OutputPort_write },
	{"__gc", OutputPort_gc },
	{NULL, NULL}
};


/***************************************************************
 * Variable (DataSourceBase)
 ***************************************************************/

static int Variable_getTypes(lua_State *L)
{
	push_vect_str(L, Types()->getTypes());
	return 1;
}

static int Variable_getMemberNames(lua_State *L)
{
	DataSourceBase::shared_ptr *dsbp = lua_userdata_cast2(L, 1, Variable, DataSourceBase::shared_ptr);
	push_vect_str(L, (*dsbp)->getMemberNames());
	return 1;
}

static int Variable_getMember(lua_State *L)
{
	DataSourceBase::shared_ptr *dsbp = lua_userdata_cast2(L, 1, Variable, DataSourceBase::shared_ptr);
	DataSourceBase::shared_ptr memdsb;
	const char *mem = luaL_checkstring(L, 2);

	memdsb = (*dsbp)->getMember(mem);

	if(memdsb == 0)
		luaL_error(L, "Variable.getMember: no member named %s ", mem);

	lua_pushobject2(L, Variable, DataSourceBase::shared_ptr)(memdsb);
	return 1;
}

static int Variable_update(lua_State *L)
{
	int ret;
	DataSourceBase::shared_ptr *self = lua_userdata_cast2(L, 1, Variable, DataSourceBase::shared_ptr);
	DataSourceBase::shared_ptr *dsbp = lua_userdata_cast2(L, 2, Variable, DataSourceBase::shared_ptr);

	ret = (*self)->update(dsbp->get());
	lua_pushboolean(L, ret);
	return 1;
}

/*
 * converts a DataSourceBase to the corresponding Lua value and pushes
 * that on the stack.
 */
static int __Variable_tolua(lua_State *L, DataSourceBase::shared_ptr dsb)
{
	DataSourceBase *ds = dsb.get();
	assert(ds);
	const std::string type = dsb->getTypeName();

	if(type=="bool") {
		DataSource<bool>* dsb = DataSource<bool>::narrow(ds);
		if(dsb) lua_pushboolean(L, dsb->get());
		else goto out_nodsb;
	} else if (type == "float") {
		DataSource<float>* dsb = DataSource<float>::narrow(ds);
		if(dsb) lua_pushnumber(L, ((lua_Number) dsb->get()));
		else goto out_nodsb;
	} else if (type == "double") {
		DataSource<double>* dsb = DataSource<double>::narrow(ds);
		if(dsb) lua_pushnumber(L, ((lua_Number) dsb->get()));
		else goto out_nodsb;
	} else if (type == "uint") {
		DataSource<unsigned int>* dsb = DataSource<unsigned int>::narrow(ds);
		if(dsb) lua_pushnumber(L, ((lua_Number) dsb->get()));
		else goto out_nodsb;
	} else if (type == "int") {
		DataSource<int>* dsb = DataSource<int>::narrow(ds);
		if(dsb) lua_pushnumber(L, ((lua_Number) dsb->get()));
		else goto out_nodsb;
	} else if (type == "char") {
		DataSource<char>* dsb = DataSource<char>::narrow(ds);
		char c = dsb->get();
		if(dsb) lua_pushlstring(L, &c, 1);
		else goto out_nodsb;
	} else if (type == "string") {
		DataSource<std::string>* dsb = DataSource<std::string>::narrow(ds);
		if(dsb) lua_pushlstring(L, dsb->get().c_str(), dsb->get().size());
		else goto out_nodsb;
	} else {
		goto out_conv_err;
	}

	/* all ok */
	return 1;

 out_conv_err:
	luaL_error(L, "Variable.tolua: can't convert type %s", type.c_str());
	return 0;

 out_nodsb:
	luaL_error(L, "Variable.tolua: narrow failed for %s Variable", type.c_str());
	return 0;
}

static int Variable_tolua(lua_State *L)
{
	DataSourceBase::shared_ptr dsb = *(lua_userdata_cast2(L, 1, Variable, DataSourceBase::shared_ptr));
	return __Variable_tolua(L, dsb);
}

/* create variable */
static int Variable_create(lua_State *L)
{
	const char *type;
	type = luaL_checkstring(L, 1);

	if(!strcmp(type, "void"))
		luaL_error(L, "Variable.new: can't create void variable");

	TypeInfo* ti = Types()->type(type);

	if(ti==0)
		luaL_error(L, "Variable.new: unknown type %s", type);

	lua_pushobject2(L, Variable, DataSourceBase::shared_ptr)(ti->buildValue());
	return 1;
}

/* create variable and initialize */
static int Variable_create_ival(lua_State *L, int typeind, int valind)
{
	DataSourceBase::shared_ptr dsb;
	luaL_checkany(L, valind);
	const char* type = luaL_checkstring(L, typeind);	/* target dsb type */
	int luatype = lua_type(L, valind);			/* type of lua variable */

	if(strcmp(type, "bool") == 0) {
		lua_Number x;
		if(luatype == LUA_TBOOLEAN)
			x = (lua_Number) lua_toboolean(L, valind);
		else if (luatype == LUA_TNUMBER)
			x = lua_tonumber(L, valind);
		else
			goto out_conv_err;

		dsb = new ValueDataSource<bool>((bool) x);
		lua_pushobject2(L, Variable, DataSourceBase::shared_ptr)(dsb);

	} else if (strcmp(type, "int") == 0) {
		lua_Number x;
		if (luatype == LUA_TNUMBER)
			x = lua_tonumber(L, valind);
		else
			goto out_conv_err;

		dsb = new ValueDataSource<int>(x); // (int)
		lua_pushobject2(L, Variable, DataSourceBase::shared_ptr)(dsb);

	} else if (strcmp(type, "uint") == 0) {
		lua_Number x;
		if (luatype == LUA_TNUMBER)
			x = lua_tonumber(L, valind);
		else
			goto out_conv_err;

		dsb = new ValueDataSource<unsigned int>((unsigned int) x);
		lua_pushobject2(L, Variable, DataSourceBase::shared_ptr)(dsb);
	} else if (strcmp(type, "double") == 0) {
		lua_Number x;
		if (luatype == LUA_TNUMBER)
			x = lua_tonumber(L, valind);
		else
			goto out_conv_err;

		dsb = new ValueDataSource<double>((double) x);
		lua_pushobject2(L, Variable, DataSourceBase::shared_ptr)(dsb);
	} else if (strcmp (type, "float") == 0) {
		lua_Number x;
		if (luatype == LUA_TNUMBER)
			x = lua_tonumber(L, valind);
		else
			goto out_conv_err;

		dsb = new ValueDataSource<float>((float) x);
		lua_pushobject2(L, Variable, DataSourceBase::shared_ptr)(dsb);
	} else if (strcmp(type, "char") == 0) {
		const char *x;
		size_t l;
		if (luatype == LUA_TSTRING)
			x = lua_tolstring(L, valind, &l);
		else
			goto out_conv_err;

		dsb = new ValueDataSource<char>(x[0]);
		lua_pushobject2(L, Variable, DataSourceBase::shared_ptr)(dsb);
	} else if (strcmp(type, "string") == 0) {
		// std::string x;
		const char *x;
		if (luatype == LUA_TSTRING)
			x = lua_tostring(L, valind); /* nonhrt */
		else
			goto out_conv_err;

		dsb = new ValueDataSource<std::string>(x);
		lua_pushobject2(L, Variable, DataSourceBase::shared_ptr)(dsb);
	} else {
		goto out_conv_err;
	}

	/* everybody happy */
	return 1;

 out_conv_err:
	luaL_error(L, "Variable.tovar: can't convert lua %s to %s variable", lua_typename(L, luatype), type);
	return 0;
}

static int Variable_new(lua_State *L)
{
	int argc = lua_gettop(L);
	if(argc == 1)
		return Variable_create(L);
	else if(argc == 2)
		return Variable_create_ival(L, 1, 2);
	else
		luaL_error(L, "Variable.new: invalid number of args");

	return 0;
}

static int Variable_toString(lua_State *L)
{
	DataSourceBase::shared_ptr *dsbp = lua_userdata_cast2(L, 1, Variable, DataSourceBase::shared_ptr);
	lua_pushstring(L, ((*dsbp)->toString()).c_str());
	return 1;
}

static int Variable_getType(lua_State *L)
{
	DataSourceBase::shared_ptr *dsbp = lua_userdata_cast2(L, 1, Variable, DataSourceBase::shared_ptr);
	lua_pushstring(L, (*dsbp)->getType().c_str());
	return 1;
}

static int Variable_getTypeName(lua_State *L)
{
	DataSourceBase::shared_ptr *dsbp = lua_userdata_cast2(L, 1, Variable, DataSourceBase::shared_ptr);
	lua_pushstring(L, (*dsbp)->getTypeName().c_str());
	return 1;
}

/*
 * Operators
 */
static int Variable_unm(lua_State *L)
{
	types::OperatorRepository::shared_ptr opreg = types::OperatorRepository::Instance();
	DataSourceBase::shared_ptr arg = *(lua_userdata_cast2(L, 1, Variable, DataSourceBase::shared_ptr));
	DataSourceBase::shared_ptr res = opreg->applyUnary("-", arg.get());
	lua_pushobject2(L, Variable, DataSourceBase::shared_ptr)(res);
	return 1;
}


/* don't try this at home */
#define gen_opmet(name, op)					\
static int name(lua_State *L)					\
{								\
	DataSourceBase::shared_ptr arg1 = *(lua_userdata_cast2(L, 1, Variable, DataSourceBase::shared_ptr)); \
	DataSourceBase::shared_ptr arg2 = *(lua_userdata_cast2(L, 2, Variable, DataSourceBase::shared_ptr)); \
	types::OperatorRepository::shared_ptr opreg = types::OperatorRepository::Instance(); \
	DataSourceBase *res = opreg->applyBinary(#op, arg1.get(), arg2.get()); \
	if(res == 0)							\
		luaL_error(L , "%s (operator %s) failed", #name, #op);	\
	res->evaluate();						\
	lua_pushobject2(L, Variable, DataSourceBase::shared_ptr)(res);	\
	return 1;							\
}									\

gen_opmet(Variable_add, +)
gen_opmet(Variable_sub, -)
gen_opmet(Variable_mul, *)
gen_opmet(Variable_div, /)
gen_opmet(Variable_mod, %)
gen_opmet(Variable_pow, ^)

/* these flavors convert the boolean return dsb to a lua bool */
#define gen_opmet_bool(name, op)				\
static int name(lua_State *L)					\
{								\
	DataSourceBase::shared_ptr arg1 = *(lua_userdata_cast2(L, 1, Variable, DataSourceBase::shared_ptr)); \
	DataSourceBase::shared_ptr arg2 = *(lua_userdata_cast2(L, 2, Variable, DataSourceBase::shared_ptr)); \
	types::OperatorRepository::shared_ptr opreg = types::OperatorRepository::Instance(); \
	DataSourceBase *res = opreg->applyBinary(#op, arg1.get(), arg2.get()); \
	if(res == 0)							\
		luaL_error(L , "%s (operator %s) failed", #name, #op);	\
	res->evaluate();						\
	return __Variable_tolua(L, res);				\
}									\

gen_opmet_bool(Variable_eq, ==)
gen_opmet_bool(Variable_lt, <)
gen_opmet_bool(Variable_le, <=)

static int Variable_opBinary(lua_State *L)
{
	types::OperatorRepository::shared_ptr opreg = types::OperatorRepository::Instance();
	const char *op = luaL_checkstring(L, 1);
	DataSourceBase::shared_ptr arg1 = *(lua_userdata_cast2(L, 2, Variable, DataSourceBase::shared_ptr));
	DataSourceBase::shared_ptr arg2 = *(lua_userdata_cast2(L, 3, Variable, DataSourceBase::shared_ptr));
	DataSourceBase *res;

	res = opreg->applyBinary(op, arg1.get(), arg2.get());
	if(res == 0)
		luaL_error(L , "Variable.opBinary '%s' not applicable to args", op);

	res->evaluate();

	lua_pushobject2(L, Variable, DataSourceBase::shared_ptr)(res);
	return 1;
}


static int Variable_opDot(lua_State *L)
{
	DataSourceBase::shared_ptr dsb = *(lua_userdata_cast2(L, 1, Variable, DataSourceBase::shared_ptr));
	const char *member = luaL_checkstring(L, 2);

	types::OperatorRepository::shared_ptr opreg = types::OperatorRepository::Instance();
	DataSourceBase *res = opreg->applyDot(member, dsb.get());

	if (res == 0) {
		luaL_error(L, "Variable.opDot: indexing failed, no member %s", member);
	} else {
		res->evaluate();
		lua_pushobject2(L, Variable, DataSourceBase::shared_ptr)(res);
	}

	return 1;
}

/*
 * this is a dispatcher which checks if the key is a method, otherwise
 * calls opDot for looking up the field. Inspired by
 * http://lua-users.org/wiki/ObjectProperties
 */
static int Variable_index(lua_State *L)
{
	const char* key = luaL_checkstring(L, 2);

	lua_getmetatable(L, 1);
	lua_getfield(L, -1, key);

	// Either key is name of a method in the metatable
	if(!lua_isnil(L, -1))
		return 1;

	// ... or its a field access, so recall as self.get(self, value).
	lua_settop(L, 2);
	return Variable_opDot(L);
}

static int Variable_newindex(lua_State *L)
{
	DataSourceBase::shared_ptr master = *(lua_userdata_cast2(L, 1, Variable, DataSourceBase::shared_ptr));
	const char* member = luaL_checkstring(L, 2);
	DataSourceBase::shared_ptr newval = *(lua_userdata_cast2(L, 3, Variable, DataSourceBase::shared_ptr));

	/* get dsb to be updated */
	types::OperatorRepository::shared_ptr opreg = types::OperatorRepository::Instance();
	DataSourceBase *curval = opreg->applyDot(member, master.get());

	lua_pushboolean(L, curval->update(newval.get()));
	return 1;
}



// static int Variable_gc(lua_State *L)
// {
// 	DataSourceBase::shared_ptr *dsbp = (DataSourceBase::shared_ptr *) luaL_checkudata(L, 1, "Variable");
// 	(*dsbp).~intrusive_ptr();
// 	return 0;
// }

static const struct luaL_Reg Variable_f [] = {
	{ "new", Variable_new },
	{ "tolua", Variable_tolua },
	{ "toString", Variable_toString },
	{ "getTypes", Variable_getTypes },
	{ "getType", Variable_getType },
	{ "getTypeName", Variable_getTypeName },
	{ "getMemberNames", Variable_getMemberNames },
	{ "getMember", Variable_getMember },
	{ "opBinary", Variable_opBinary },
	{ "opDot", Variable_opDot },
	{ "update", Variable_update },
	{ "unm", Variable_unm },
	{ "add", Variable_add },
	{ "sub", Variable_sub },
	{ "mul", Variable_mul },
	{ "div", Variable_div },
	{ "mod", Variable_mod },
	{ "pow", Variable_pow },
	{ "eq", Variable_eq },
	{ "lt", Variable_lt },
	{ "le", Variable_le },
	{ NULL, NULL}
};

static const struct luaL_Reg Variable_m [] = {
	{ "tolua", Variable_tolua },
	{ "toString", Variable_toString },
	{ "getType", Variable_getType },
	{ "getTypeName", Variable_getTypeName },
	{ "getMemberNames", Variable_getMemberNames },
	{ "getMember", Variable_getMember },
	{ "opBinary", Variable_opBinary },
	{ "opDot", Variable_opDot },
	{ "update", Variable_update },
	{ "__unm", Variable_unm },
	{ "__add", Variable_add },
	{ "__sub", Variable_sub },
	{ "__mul", Variable_mul },
	{ "__div", Variable_div },
	{ "__mod", Variable_mod },
	{ "__pow", Variable_pow },
	{ "__eq", Variable_eq },
	{ "__lt", Variable_lt },
	{ "__le", Variable_le },
	{ "__index", Variable_index },
	{ "__newindex", Variable_newindex },
	{ "__gc", GCMethod<DataSourceBase::shared_ptr> },
	// {"__gc", Variable_gc},
	{ NULL, NULL}
};


/***************************************************************
 * TaskContext (boxed)
 ***************************************************************/
static int TaskContext_getName(lua_State *L)
{
	const char *s;
	TaskContext *tc = *(lua_getudata_bx(L, 1, TaskContext));
	s = tc->getName().c_str();
	lua_pushstring(L, s);
	return 1;
}

static int TaskContext_start(lua_State *L)
{
	TaskContext *tc = *(lua_getudata_bx(L, 1, TaskContext));
	bool b = tc->start();
	lua_pushboolean(L, b);
	return 1;
}

static int TaskContext_stop(lua_State *L)
{
	TaskContext *tc = *(lua_getudata_bx(L, 1, TaskContext));
	bool b = tc->stop();
	lua_pushboolean(L, b);
	return 1;
}

static int TaskContext_configure(lua_State *L)
{
	TaskContext *tc = *(lua_getudata_bx(L, 1, TaskContext));
	bool ret = tc->configure();
	lua_pushboolean(L, ret);
	return 1;
}

static int TaskContext_activate(lua_State *L)
{
	TaskContext *tc = *(lua_getudata_bx(L, 1, TaskContext));
	bool ret = tc->activate();
	lua_pushboolean(L, ret);
	return 1;
}

static int TaskContext_cleanup(lua_State *L)
{
	TaskContext *tc = *(lua_getudata_bx(L, 1, TaskContext));
	bool ret = tc->cleanup();
	lua_pushboolean(L, ret);
	return 1;
}

static int TaskContext_getState(lua_State *L)
{
	enum TaskCore::TaskState ts;
	TaskContext **tc = (TaskContext**) lua_getudata_bx(L, 1, TaskContext);
	ts = (*tc)->getTaskState();

	switch(ts) {
	case TaskCore::Init:
		lua_pushstring(L, "Init"); break;
	case TaskCore::PreOperational:
		lua_pushstring(L, "PreOperational"); break;
	case TaskCore::FatalError:
		lua_pushstring(L, "FatalError"); break;
	case TaskCore::Exception:
		lua_pushstring(L, "Exception"); break;
	case TaskCore::Stopped:
		lua_pushstring(L, "Stopped"); break;
	case TaskCore::Running:
		lua_pushstring(L, "Running"); break;
	case TaskCore::RunTimeError:
		lua_pushstring(L, "RunTimeError"); break;
	}
	return 1;
}

/* string-table getPeers(TaskContext self)*/
/* should better return array of TC's */
static int TaskContext_getPeers(lua_State *L)
{
	TaskContext *tc = *(lua_getudata_bx(L, 1, TaskContext));
	std::vector<std::string> plist = tc->getPeerList();
	push_vect_str(L, plist);
	return 1;
}

/* bool addPeer(TaskContext self, TaskContext peer)*/
static int TaskContext_addPeer(lua_State *L)
{
	bool ret;
	TaskContext *self = *(lua_getudata_bx(L, 1, TaskContext));
	TaskContext *peer = *(lua_getudata_bx(L, 2, TaskContext));
	ret = self->addPeer(peer);
	lua_pushboolean(L, ret);
	return 1;
}

/* void removePeer(TaskContext self, string peer)*/
static int TaskContext_removePeer(lua_State *L)
{
	std::string peer;
	TaskContext *self = *(lua_getudata_bx(L, 1, TaskContext));
	peer = luaL_checkstring(L, 2);
	self->removePeer(peer);
	return 0;
}

/* TaskContext getPeer(string name) */
static int TaskContext_getPeer(lua_State *L)
{
	std::string strpeer;
	TaskContext *peer;
	TaskContext **tc; /* boxed TC */
	TaskContext *self = *(lua_getudata_bx(L, 1, TaskContext));
	strpeer = luaL_checkstring(L, 2);
	peer = self->getPeer(strpeer);

	tc = (TaskContext**) lua_newuserdata(L, sizeof(TaskContext*));
	*tc = peer;
	luaL_getmetatable(L, "TaskContext");
	lua_setmetatable(L, -2);
	return 1;
}

static int TaskContext_getPortNames(lua_State *L)
{
	TaskContext *tc = *(lua_getudata_bx(L, 1, TaskContext));
	std::vector<std::string> plist = tc->ports()->getPortNames();
	push_vect_str(L, plist);
	return 1;
}

static int TaskContext_addPort(lua_State *L)
{
	PortInterface **pi;
	TaskContext *tc = *(lua_getudata_bx(L, 1, TaskContext));

	if((pi = (PortInterface**) luaL_testudata(L, 2, "InputPort")) != NULL)
		tc->ports()->addPort(**pi);
	else if((pi = (PortInterface**) luaL_testudata(L, 2, "OutputPort")) != NULL)
		tc->ports()->addPort(**pi);
	else
		luaL_error(L, "addPort: invalid argument, not a Port");
 	return 0;
}

static int TaskContext_addEventPort(lua_State *L)
{
	InputPortInterface **ipi;
	TaskContext *tc = *(lua_getudata_bx(L, 1, TaskContext));

	if((ipi = (InputPortInterface**) luaL_testudata(L, 2, "InputPort")) != NULL)
		tc->ports()->addEventPort(**ipi);
	else
		luaL_error(L, "addEventPort: invalid argument, not an InputPort");
 	return 0;
}

static int TaskContext_addProperty(lua_State *L)
{
	int ret;
	TaskContext *tc = *(lua_getudata_bx(L, 1, TaskContext));
	PropertyBase *pb = *(lua_getudata_bx2(L, 2, Property, PropertyBase));
	ret = tc->addProperty(*pb);
	lua_pushboolean(L, ret);
	return 1;
}

static int TaskContext_getProperty(lua_State *L)
{
	const char *name;
	PropertyBase *prop;
	PropertyBase **pb;

	TaskContext *tc = *(lua_getudata_bx(L, 1, TaskContext));
	name = luaL_checkstring(L, 2);

	prop = tc->getProperty(name);

	if(!prop)
		luaL_error(L, "%s failed. No such property", __FILE__);

	pb = (PropertyBase**) lua_newuserdata(L, sizeof(PropertyBase*));
	*pb = prop;
	luaL_getmetatable(L, "Property");
	lua_setmetatable(L, -2);
	return 1;
}

static int TaskContext_getProperties(lua_State *L)
{
	PropertyBase **pb;
	TaskContext *tc = *(lua_getudata_bx(L, 1, TaskContext));
	vector<PropertyBase*> props = tc->properties()->getProperties();

	int key = 1;
	lua_createtable(L, props.size(), 0);
	for(vector<PropertyBase*>::iterator it = props.begin(); it != props.end(); ++it) {
		pb = (PropertyBase**) lua_newuserdata(L, sizeof(PropertyBase*));
		*pb = *it;
		luaL_getmetatable(L, "Property");
		lua_setmetatable(L, -2);
		lua_rawseti(L, -2, key++);
	}

	return 1;
}


static int TaskContext_getOps(lua_State *L)
{
	TaskContext *tc = *(lua_getudata_bx(L, 1, TaskContext));
	std::vector<std::string> oplst = tc->operations()->getNames();
	push_vect_str(L, oplst);
	return 1;
}

/* returns restype, arity, table-of-arg-descr */
static int TaskContext_getOpInfo(lua_State *L)
{
	int i=1;
	TaskContext *tc = *(lua_getudata_bx(L, 1, TaskContext));
	const char *op = luaL_checkstring(L, 2);
	std::vector<ArgumentDescription> args;

	lua_pushstring(L, tc->operations()->getResultType(op).c_str()); /* result type */
	lua_pushinteger(L, tc->operations()->getArity(op));		/* arity */

	args = tc->operations()->getArgumentList(op);

	lua_newtable(L);

	for (std::vector<ArgumentDescription>::iterator it = args.begin(); it != args.end(); it++) {
		lua_newtable(L);
		lua_pushstring(L, "name"); lua_pushstring(L, it->name.c_str()); lua_rawset(L, -3);
		lua_pushstring(L, "type"); lua_pushstring(L, it->type.c_str()); lua_rawset(L, -3);
		lua_pushstring(L, "desc"); lua_pushstring(L, it->description.c_str()); lua_rawset(L, -3);
		lua_rawseti(L, -2, i++);
	}

	return 3;
}


static int TaskContext_call(lua_State *L)
{
	unsigned int argc = lua_gettop(L);
	TaskContext *tc = *(lua_getudata_bx(L, 1, TaskContext));
	const char *op = luaL_checkstring(L, 2);
	std::vector<base::DataSourceBase::shared_ptr> args;
	DataSourceBase::shared_ptr *dsbp;
	base::DataSourceBase::shared_ptr ret;

	OperationInterfacePart *orp = tc->operations()->getPart(op);

	if(!orp)
		luaL_error(L, "TaskContext.call: no operation %s", op);

	if(orp->arity() != argc-2)
		luaL_error(L, "TaskContext.call: wrong number of args. expected %d, got %d", orp->arity(), argc);

	for(unsigned int arg=3; arg<=argc; arg++) {
		dsbp = lua_userdata_cast2(L, arg, Variable, DataSourceBase::shared_ptr);
		/* tbd: coerce reasonably */
		args.push_back(*dsbp);
	}

	ret = tc->operations()->produce(op, args, NULL);
	ret->evaluate();
	lua_pushobject2(L, Variable, DataSourceBase::shared_ptr)(ret);
	return 1;
}

/* only explicit destruction allowed */
static int TaskContext_del(lua_State *L)
{
	TaskContext *tc = *(lua_getudata_bx(L, 1, TaskContext));
	tc->~TaskContext();

	/* this prevents calling rtt methods which would cause a crash */
	luaL_getmetatable(L, "__dead__");
	lua_setmetatable(L, -2);
	return 0;
}


static const struct luaL_Reg TaskContext_f [] = {
	{ "getName", TaskContext_getName },
	{ "start", TaskContext_start },
	{ "stop", TaskContext_stop },
	{ "configure", TaskContext_configure },
	{ "activate", TaskContext_activate },
	{ "cleanup", TaskContext_cleanup },
	{ "getState", TaskContext_getState },
	{ "getPeers", TaskContext_getPeers },
	{ "addPeer", TaskContext_addPeer },
	{ "removePeer", TaskContext_removePeer },
	{ "getPeer", TaskContext_getPeer },
	{ "getPortNames", TaskContext_getPortNames },
	{ "addPort", TaskContext_addPort },
	{ "addEventPort", TaskContext_addEventPort },
	{ "addProperty", TaskContext_addProperty },
	{ "getProperty", TaskContext_getProperty },
	{ "getProperties", TaskContext_getProperties },
	{ "getOps", TaskContext_getOps },
	{ "getOpInfo", TaskContext_getOpInfo },
	{ "call", TaskContext_call },
	{ "delete", TaskContext_del },
	{ NULL, NULL}
};

static const struct luaL_Reg TaskContext_m [] = {
	{ "getName", TaskContext_getName },
	{ "start", TaskContext_start },
	{ "stop", TaskContext_stop },
	{ "configure", TaskContext_configure },
	{ "activate", TaskContext_activate },
	{ "cleanup", TaskContext_cleanup },
	{ "getState", TaskContext_getState },
	{ "getPeers", TaskContext_getPeers },
	{ "addPeer", TaskContext_addPeer },
	{ "removePeer", TaskContext_removePeer },
	{ "getPeer", TaskContext_getPeer },
	{ "getPortNames", TaskContext_getPortNames },
	{ "addPort", TaskContext_addPort },
	{ "addEventPort", TaskContext_addEventPort },
	{ "addProperty", TaskContext_addProperty },
	{ "getProperty", TaskContext_getProperty },
	{ "getProperties", TaskContext_getProperties },
	{ "getOps", TaskContext_getOps },
	{ "getOpInfo", TaskContext_getOpInfo },
	{ "call", TaskContext_call },
	/* we don't GC TaskContexts
	 * { "__gc", GCMethod<TaskContext> }, */
	{ NULL, NULL}
};

/*
 * Logger
 */
static const char *const loglevels[] = {
	"Never", "Fatal", "Critical", "Error", "Warning", "Info", "Debug", "RealTime", NULL
};

static int Logger_setlevel(lua_State *L)
{
	Logger::LogLevel ll = (Logger::LogLevel) luaL_checkoption(L, 1, NULL, loglevels);
	log().setLogLevel(ll);
	return 0;
}

static int Logger_getlevel(lua_State *L)
{
	Logger::LogLevel ll = log().getLogLevel();

	switch(ll) {
	case Logger::Never:	lua_pushstring(L, "Never"); break;
	case Logger::Fatal:	lua_pushstring(L, "Fatal"); break;
	case Logger::Critical:	lua_pushstring(L, "Critical"); break;
	case Logger::Error:	lua_pushstring(L, "Error"); break;
	case Logger::Warning:	lua_pushstring(L, "Warning"); break;
	case Logger::Info: 	lua_pushstring(L, "Info"); break;
	case Logger::Debug:	lua_pushstring(L, "Debug"); break;
	case Logger::RealTime:	lua_pushstring(L, "RealTime"); break;
	default:
		lua_pushstring(L, "unknown");
	}
	return 1;
}

static int Logger_log(lua_State *L)
{
	const char *mes;
	for(int i=1; i<=lua_gettop(L); i++) {
		mes = luaL_checkstring(L, i);
		Logger::log() << mes;
	}
	Logger::log() << endlog();
	return 0;
}

static const struct luaL_Reg Logger_f [] = {
	{"setlevel", Logger_setlevel },
	{"getlevel", Logger_getlevel },
	{"log", Logger_log },
	{NULL, NULL}
};


extern "C" int luaopen_rtt(lua_State *L);

int luaopen_rtt(lua_State *L)
{
	luaL_newmetatable(L, "__dead__");

	/* register MyObj
	 * 1. line creates metatable MyObj and registers name in registry
	 * 2. line duplicates metatable
	 * 3. line sets metatable[__index]=metatable
	 *    (more precisely: table at -2 [__index] = top_of_stack, pops top of stack)
	 * 4. line register methods in metatable
	 * 5. line registers free functions in global mystuff.MyObj table
	 */
	luaL_newmetatable(L, "TaskContext");
	lua_pushvalue(L, -1); /* duplicates metatable */
	lua_setfield(L, -2, "__index");
	luaL_register(L, NULL, TaskContext_m);
	luaL_register(L, "rtt.TaskContext", TaskContext_f);

	luaL_newmetatable(L, "InputPort");
	lua_pushvalue(L, -1); /* duplicates metatable */
	lua_setfield(L, -2, "__index");
	luaL_register(L, NULL, InputPort_m);
	luaL_register(L, "rtt.InputPort", InputPort_f);

	luaL_newmetatable(L, "OutputPort");
	lua_pushvalue(L, -1); /* duplicates metatable */
	lua_setfield(L, -2, "__index");
	luaL_register(L, NULL, OutputPort_m);
	luaL_register(L, "rtt.OutputPort", OutputPort_f);

	luaL_newmetatable(L, "Variable");
	lua_pushvalue(L, -1); /* duplicates metatable */
	lua_setfield(L, -2, "__index");
	luaL_register(L, NULL, Variable_m);
	luaL_register(L, "rtt.Variable", Variable_f);

	luaL_newmetatable(L, "Property");
	lua_pushvalue(L, -1); /* duplicates metatable */
	lua_setfield(L, -2, "__index");
	luaL_register(L, NULL, Property_m);
	luaL_register(L, "rtt.Property", Property_f);

	/* only functions */
	luaL_newmetatable(L, "Logger");
	luaL_register(L, "rtt.Logger", Logger_f);

	return 1;
}
