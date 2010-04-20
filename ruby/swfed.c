#include "ruby.h"

VALUE swfed_swfInfo ( VALUE self )
{
    return Qnil;
}

void Init_swfed ( void )
{
    VALUE c;

    c = rb_define_class( "SWFEditor", rb_cObject );
    rb_define_method( c, "swfInfo", swfed_swfInfo, 0 );
}
