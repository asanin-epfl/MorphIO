/* Copyright (c) 2013-2017, EPFL/Blue Brain Project
 *                          Juan Hernando <jhernando@fi.upm.es>
 *
 * This file is part of Brion <https://github.com/BlueBrain/Brion>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "soma.h"
#include "morphologyImpl.h"
#include "section.h"

namespace morphio
{
namespace
{
Point _computeCentroid(const Points& points)
{
    Point centroid;
    for (const Point& point : points)
        centroid += point;
    centroid /= float(points.size());
    return centroid;
}
}

Soma::Soma(Morphology::ImplPtr morphology)
    : _morphology(morphology)
{
}

Soma::Soma(const Soma& soma)
    : _morphology(soma._morphology)
{
}

Soma& Soma::operator=(const Soma& soma)
{
    if (&soma == this)
        return *this;
    _morphology = soma._morphology;
    return *this;
}

Points Soma::getProfilePoints() const
{
    return _morphology->getSectionSamples(_morphology->somaSection);
}

// float Soma::getMeanRadius() const
// {
//     const Points points = getProfilePoints();
//     const Point centroid = _computeCentroid(points);
//     float radius = 0;
//     for (const Point point : points)
//         radius += (point - centroid).length();
//     return radius /= float(points.size());
// }

Point Soma::getCentroid() const
{
    return _computeCentroid(getProfilePoints());
}

Sections Soma::getChildren() const
{
    const uint32_ts& children =
        _morphology->getChildren(_morphology->somaSection);
    Sections result;
    for (const uint32_t id : children)
        result.push_back(Section(id, _morphology));
    return result;
}
}
