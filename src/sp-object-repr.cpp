#define __SP_OBJECT_REPR_C__

/*
 * Object type dictionary and build frontend
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2003 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-defs.h"
#include "sp-symbol.h"
#include "sp-marker.h"
#include "sp-use.h"
#include "sp-root.h"
#include "sp-image.h"
#include "sp-linear-gradient-fns.h"
#include "sp-path.h"
#include "sp-radial-gradient-fns.h"
#include "sp-rect.h"
#include "sp-ellipse.h"
#include "sp-star.h"
#include "sp-stop-fns.h"
#include "sp-spiral.h"
#include "sp-offset.h"
#include "sp-line.h"
#include "sp-metadata.h"
#include "sp-polyline.h"
#include "sp-textpath.h"
#include "sp-tspan.h"
#include "sp-pattern.h"
#include "sp-clippath.h"
#include "sp-mask.h"
#include "sp-anchor.h"
//#include "sp-animation.h"
#include "sp-flowdiv.h"
#include "sp-flowregion.h"
#include "sp-flowtext.h"
#include "sp-style-elem.h"
#include "sp-switch.h"
#include "color-profile-fns.h"
#include "xml/repr.h"
#include "sp-filter.h"
#include "sp-gaussian-blur.h"
#include "sp-feblend.h"
#include "sp-fecolormatrix.h"
#include "sp-fecomponenttransfer.h"
#include "sp-fecomposite.h"
#include "sp-feconvolvematrix.h"
#include "sp-fediffuselighting.h"
#include "sp-fedisplacementmap.h"
#include "sp-feflood.h"
#include "sp-feimage.h"
#include "sp-femerge.h"
#include "sp-femorphology.h"
#include "sp-feoffset.h"
#include "sp-fespecularlighting.h"
#include "sp-fetile.h"
#include "sp-feturbulence.h"

enum NameType { REPR_NAME, SODIPODI_TYPE };
static unsigned const N_NAME_TYPES = SODIPODI_TYPE + 1;

static GType name_to_gtype(NameType name_type, gchar const *name);

/**
 * Construct an SPRoot and all its descendents from the given repr.
 */
SPObject *
sp_object_repr_build_tree(SPDocument *document, Inkscape::XML::Node *repr)
{
    g_assert(document != NULL);
    g_assert(repr != NULL);

    gchar const * const name = repr->name();
    g_assert(name != NULL);
    GType const type = name_to_gtype(REPR_NAME, name);
    g_assert(g_type_is_a(type, SP_TYPE_ROOT));
    gpointer newobj = g_object_new(type, 0);
    g_assert(newobj != NULL);
    SPObject *const object = SP_OBJECT(newobj);
    g_assert(object != NULL);
    sp_object_invoke_build(object, document, repr, FALSE);

    return object;
}

GType
sp_repr_type_lookup(Inkscape::XML::Node *repr)
{
    if ( repr->type() == Inkscape::XML::TEXT_NODE ) {
        return SP_TYPE_STRING;
    } else if ( repr->type() == Inkscape::XML::ELEMENT_NODE ) {
        gchar const * const type_name = repr->attribute("sodipodi:type");
        return ( type_name
                 ? name_to_gtype(SODIPODI_TYPE, type_name)
                 : name_to_gtype(REPR_NAME, repr->name()) );
    } else {
        return 0;
    }
}

static GHashTable *t2dtable[N_NAME_TYPES] = {NULL};

static void
populate_dtables()
{
    struct NameTypeEntry { char const *const name; GType const type_id; };
    NameTypeEntry const repr_name_entries[] = {
        { "svg:a", SP_TYPE_ANCHOR },
        //{ "svg:animate", SP_TYPE_ANIMATE },
        { "svg:circle", SP_TYPE_CIRCLE },
        { "svg:color-profile", COLORPROFILE_TYPE },
        { "svg:clipPath", SP_TYPE_CLIPPATH },
        { "svg:defs", SP_TYPE_DEFS },
        { "svg:ellipse", SP_TYPE_ELLIPSE },
        { "svg:filter", SP_TYPE_FILTER },
        /* Note: flow* elements are proposed additions for SVG 1.2, they aren't in
           SVG 1.1. */
        { "svg:flowDiv", SP_TYPE_FLOWDIV },
        { "svg:flowLine", SP_TYPE_FLOWLINE },
        { "svg:flowPara", SP_TYPE_FLOWPARA },
        { "svg:flowRegion", SP_TYPE_FLOWREGION },
        { "svg:flowRegionBreak", SP_TYPE_FLOWREGIONBREAK },
        { "svg:flowRegionExclude", SP_TYPE_FLOWREGIONEXCLUDE },
        { "svg:flowRoot", SP_TYPE_FLOWTEXT },
        { "svg:flowSpan", SP_TYPE_FLOWTSPAN },
        { "svg:g", SP_TYPE_GROUP },
        { "feBlend", SP_TYPE_FEBLEND },
        { "feColorMatrix", SP_TYPE_FECOLORMATRIX },
        { "feComponentTransfer", SP_TYPE_FECOMPONENTTRANSFER },
        { "feComposite", SP_TYPE_FECOMPOSITE },
        { "feConvolveMatrix", SP_TYPE_FECONVOLVEMATRIX },
        { "feDiffuseLighting", SP_TYPE_FEDIFFUSELIGHTING },
        { "feDisplacementMap", SP_TYPE_FEDISPLACEMENTMAP },
        { "feFlood", SP_TYPE_FEFLOOD },
        { "feGaussianBlur", SP_TYPE_GAUSSIANBLUR },
        { "feImage", SP_TYPE_FEIMAGE },
        { "feMerge", SP_TYPE_FEMERGE },
        { "feMorphology", SP_TYPE_FEMORPHOLOGY },
        { "feOffset", SP_TYPE_FEOFFSET },
        { "feSpecularLighting", SP_TYPE_FESPECULARLIGHTING },
        { "feTile", SP_TYPE_FETILE },
        { "feTurbulence", SP_TYPE_FETURBULENCE },
        { "svg:image", SP_TYPE_IMAGE },
        { "svg:line", SP_TYPE_LINE },
        { "svg:linearGradient", SP_TYPE_LINEARGRADIENT },
        { "svg:marker", SP_TYPE_MARKER },
        { "svg:mask", SP_TYPE_MASK },
        { "svg:metadata", SP_TYPE_METADATA },
        { "svg:path", SP_TYPE_PATH },
        { "svg:pattern", SP_TYPE_PATTERN },
        { "svg:polygon", SP_TYPE_POLYGON },
        { "svg:polyline", SP_TYPE_POLYLINE },
        { "svg:radialGradient", SP_TYPE_RADIALGRADIENT },
        { "svg:rect", SP_TYPE_RECT },
        { "svg:stop", SP_TYPE_STOP },
        { "svg:svg", SP_TYPE_ROOT },
        { "svg:style", SP_TYPE_STYLE_ELEM },
        { "svg:switch", SP_TYPE_SWITCH },
        { "svg:symbol", SP_TYPE_SYMBOL },
        { "svg:text", SP_TYPE_TEXT },
        { "svg:textPath", SP_TYPE_TEXTPATH },
        { "svg:tspan", SP_TYPE_TSPAN },
        { "svg:use", SP_TYPE_USE }
    };
    NameTypeEntry const sodipodi_name_entries[] = {
        { "arc", SP_TYPE_ARC },
        { "inkscape:offset", SP_TYPE_OFFSET },
        { "spiral", SP_TYPE_SPIRAL },
        { "star", SP_TYPE_STAR }
    };

    NameTypeEntry const *const t2entries[] = {
        repr_name_entries,
        sodipodi_name_entries
    };
    unsigned const t2n_entries[] = {
        G_N_ELEMENTS(repr_name_entries),
        G_N_ELEMENTS(sodipodi_name_entries)
    };

    for (unsigned nt = 0; nt < N_NAME_TYPES; ++nt) {
        NameTypeEntry const *const entries = t2entries[nt];
        unsigned const n_entries = t2n_entries[nt];
        GHashTable *&dtable = t2dtable[nt];

        dtable = g_hash_table_new(g_str_hash, g_str_equal);
        for (unsigned i = 0; i < n_entries; ++i) {
            g_hash_table_insert(dtable,
                                (void *)entries[i].name,
                                (gpointer) entries[i].type_id);
        }
    }
}

static inline void
ensure_dtables_populated()
{
    if (!*t2dtable) {
        populate_dtables();
    }
}

static GType
name_to_gtype(NameType const name_type, gchar const *name)
{
    ensure_dtables_populated();

    gpointer const data = g_hash_table_lookup(t2dtable[name_type], name);
    return ( ( data == NULL )
             ? SP_TYPE_OBJECT
             : (GType) data );
}

void
sp_object_type_register(gchar const *name, GType const gtype)
{
    GType const current = name_to_gtype(REPR_NAME, name);
    if (current == SP_TYPE_OBJECT) {
        g_hash_table_insert(t2dtable[REPR_NAME],
                            const_cast<gchar *>(name),
                            (gpointer) gtype);
    } else {
        /* Already registered. */
        if (current != gtype) {
            g_warning("repr type `%s' already registered as type #%lu, ignoring attempt to re-register as #%lu.",
                      name, current, gtype);
        }
    }
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
