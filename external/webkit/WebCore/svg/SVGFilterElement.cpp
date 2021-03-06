

#include "config.h"

#if ENABLE(SVG) && ENABLE(FILTERS)
#include "SVGFilterElement.h"

#include "Attr.h"
#include "FloatSize.h"
#include "MappedAttribute.h"
#include "PlatformString.h"
#include "SVGFilterBuilder.h"
#include "SVGFilterPrimitiveStandardAttributes.h"
#include "SVGLength.h"
#include "SVGNames.h"
#include "SVGParserUtilities.h"
#include "SVGResourceFilter.h"
#include "SVGUnitTypes.h"

namespace WebCore {

char SVGFilterResXIdentifier[] = "SVGFilterResX";
char SVGFilterResYIdentifier[] = "SVGFilterResY";

SVGFilterElement::SVGFilterElement(const QualifiedName& tagName, Document* doc)
    : SVGStyledElement(tagName, doc)
    , SVGURIReference()
    , SVGLangSpace()
    , SVGExternalResourcesRequired()
    , m_filterUnits(SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX)
    , m_primitiveUnits(SVGUnitTypes::SVG_UNIT_TYPE_USERSPACEONUSE)
    , m_x(LengthModeWidth, "-10%")
    , m_y(LengthModeHeight, "-10%")
    , m_width(LengthModeWidth, "120%")
    , m_height(LengthModeHeight, "120%")
{
    // Spec: If the x/y attribute is not specified, the effect is as if a value of "-10%" were specified.
    // Spec: If the width/height attribute is not specified, the effect is as if a value of "120%" were specified.
}

SVGFilterElement::~SVGFilterElement()
{
}

void SVGFilterElement::setFilterRes(unsigned long, unsigned long) const
{
}

void SVGFilterElement::parseMappedAttribute(MappedAttribute* attr)
{
    const String& value = attr->value();
    if (attr->name() == SVGNames::filterUnitsAttr) {
        if (value == "userSpaceOnUse")
            setFilterUnitsBaseValue(SVGUnitTypes::SVG_UNIT_TYPE_USERSPACEONUSE);
        else if (value == "objectBoundingBox")
            setFilterUnitsBaseValue(SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX);
    } else if (attr->name() == SVGNames::primitiveUnitsAttr) {
        if (value == "userSpaceOnUse")
            setPrimitiveUnitsBaseValue(SVGUnitTypes::SVG_UNIT_TYPE_USERSPACEONUSE);
        else if (value == "objectBoundingBox")
            setPrimitiveUnitsBaseValue(SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX);
    } else if (attr->name() == SVGNames::xAttr)
        setXBaseValue(SVGLength(LengthModeWidth, value));
    else if (attr->name() == SVGNames::yAttr)
        setYBaseValue(SVGLength(LengthModeHeight, value));
    else if (attr->name() == SVGNames::widthAttr)
        setWidthBaseValue(SVGLength(LengthModeWidth, value));
    else if (attr->name() == SVGNames::heightAttr)
        setHeightBaseValue(SVGLength(LengthModeHeight, value));
    else if (attr->name() == SVGNames::filterResAttr) {
        float x, y;
        if (parseNumberOptionalNumber(value, x, y)) {
            setFilterResXBaseValue(x);
            setFilterResYBaseValue(y);
        }
    } else {
        if (SVGURIReference::parseMappedAttribute(attr))
            return;
        if (SVGLangSpace::parseMappedAttribute(attr))
            return;
        if (SVGExternalResourcesRequired::parseMappedAttribute(attr))
            return;

        SVGStyledElement::parseMappedAttribute(attr);
    }
}

void SVGFilterElement::synchronizeProperty(const QualifiedName& attrName)
{
    SVGStyledElement::synchronizeProperty(attrName);

    if (attrName == anyQName()) {
        synchronizeX();
        synchronizeY();
        synchronizeWidth();
        synchronizeHeight();
        synchronizeFilterUnits();
        synchronizePrimitiveUnits();
        synchronizeFilterResX();
        synchronizeFilterResY();
        synchronizeExternalResourcesRequired();
        synchronizeHref();
        return;
    }

    if (attrName == SVGNames::xAttr)
        synchronizeX();
    else if (attrName == SVGNames::yAttr)
        synchronizeY();
    else if (attrName == SVGNames::widthAttr)
        synchronizeWidth();
    else if (attrName == SVGNames::heightAttr)
        synchronizeHeight();
    else if (attrName == SVGNames::filterUnitsAttr)
        synchronizeFilterUnits();
    else if (attrName == SVGNames::primitiveUnitsAttr)
        synchronizePrimitiveUnits();
    else if (attrName == SVGNames::filterResAttr) {
        synchronizeFilterResX();
        synchronizeFilterResY();
    } else if (SVGExternalResourcesRequired::isKnownAttribute(attrName))
        synchronizeExternalResourcesRequired();
    else if (SVGURIReference::isKnownAttribute(attrName))
        synchronizeHref();
}

FloatRect SVGFilterElement::filterBoundingBox(const FloatRect& objectBoundingBox) const
{
    FloatRect filterBBox;
    if (filterUnits() == SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX)
        filterBBox = FloatRect(x().valueAsPercentage() * objectBoundingBox.width() + objectBoundingBox.x(),
                               y().valueAsPercentage() * objectBoundingBox.height() + objectBoundingBox.y(),
                               width().valueAsPercentage() * objectBoundingBox.width(),
                               height().valueAsPercentage() * objectBoundingBox.height());
    else
        filterBBox = FloatRect(x().value(this),
                               y().value(this),
                               width().value(this),
                               height().value(this));

    return filterBBox;
}

void SVGFilterElement::buildFilter(const FloatRect& targetRect) const
{
    bool filterBBoxMode = filterUnits() == SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX;
    bool primitiveBBoxMode = primitiveUnits() == SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX;

    FloatRect filterBBox;
    if (filterBBoxMode)
        filterBBox = FloatRect(x().valueAsPercentage(),
                               y().valueAsPercentage(),
                               width().valueAsPercentage(),
                               height().valueAsPercentage());
    else
        filterBBox = FloatRect(x().value(this),
                               y().value(this),
                               width().value(this),
                               height().value(this));

    FloatRect filterRect = filterBBox;
    if (filterBBoxMode)
        filterRect = FloatRect(targetRect.x() + filterRect.x() * targetRect.width(),
                               targetRect.y() + filterRect.y() * targetRect.height(),
                               filterRect.width() * targetRect.width(),
                               filterRect.height() * targetRect.height());

    m_filter->setFilterBoundingBox(filterRect);
    m_filter->setFilterRect(filterBBox);
    m_filter->setEffectBoundingBoxMode(primitiveBBoxMode);
    m_filter->setFilterBoundingBoxMode(filterBBoxMode);

    if (hasAttribute(SVGNames::filterResAttr)) {
        m_filter->setHasFilterResolution(true);
        m_filter->setFilterResolution(FloatSize(filterResX(), filterResY()));
    }

    // Add effects to the filter
    m_filter->builder()->clearEffects();
    for (Node* n = firstChild(); n != 0; n = n->nextSibling()) {
        SVGElement* element = 0;
        if (n->isSVGElement()) {
            element = static_cast<SVGElement*>(n);
            if (element->isFilterEffect()) {
                SVGFilterPrimitiveStandardAttributes* effectElement = static_cast<SVGFilterPrimitiveStandardAttributes*>(element);
                if (!effectElement->build(m_filter.get())) {
                    m_filter->builder()->clearEffects();
                    break;
                }
            }
        }
    }
}

SVGResource* SVGFilterElement::canvasResource(const RenderObject*)
{
    if (!attached())
        return 0;

    if (!m_filter)
        m_filter = SVGResourceFilter::create(this);
    return m_filter.get();
}

}

#endif // ENABLE(SVG) && ENABLE(FILTERS)
