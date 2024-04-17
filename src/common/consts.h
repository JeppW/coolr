#ifndef CONSTS_H
#define CONSTS_H

#include <string>

namespace Constants {
    constexpr int MaxStringSize = 1025;
    constexpr int NumObjHeaders = 5;
    constexpr int WordSize = 4;
}

namespace Strings {
    const std::string Self = "self";

    namespace Types {
        const std::string Object = "Object";
        const std::string IO = "IO";
        const std::string Int = "Int";
        const std::string Bool = "Bool";
        const std::string String = "String";
        const std::string SelfType = "SELF_TYPE";
        const std::string NoType = "_no_type";
        const std::string PrimSlot = "prim_slot";
        const std::string MainClass = "Main";
    }

    namespace Methods {
        const std::string Abort = "abort";
        const std::string TypeName = "type_name";
        const std::string Copy = "copy";
        const std::string OutString = "out_string";
        const std::string OutInt = "out_int";
        const std::string InString = "in_string";
        const std::string InInt = "in_int";
        const std::string Length = "length";
        const std::string Concat = "concat";
        const std::string Substr = "substr";
        const std::string MainMethod = "main";
    }

    namespace Attributes {
        const std::string Val = "val";
        const std::string StrField = "str_field";
    }

    namespace Parameters {
        const std::string Arg = "arg";
        const std::string Arg1 = "arg1";
        const std::string Arg2 = "arg2";
    }
}


#endif