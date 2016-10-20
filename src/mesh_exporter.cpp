#include "mesh_exporter.hpp"

#include <algorithm>
#include <vector>

#include <hadoken/format/format.hpp>

#include <morpho/morpho_h5_v1.hpp>

using namespace std;
namespace fmt = hadoken::format;


namespace morpho{

const std::string gmsh_header =
"/***************************************************************\n"
" * gmsh file generated by morpho-tool\n"
"****************************************************************/\n\n";



std::size_t gmsh_abstract_file::add_point(const gmsh_point &point){
    gmsh_point new_point(point);
    new_point.id = _points.size();
   //std::cout << "points " << geo::get_x(point.coords ) << " " << geo::get_y(point.coords ) << " " << geo::get_z(point.coords ) << std::endl;
    return _points.insert(new_point).first->id;
}

std::size_t gmsh_abstract_file::find_point(const gmsh_point &point){
    auto it = _points.find(point);
    if(it == _points.end()){
        throw std::out_of_range(fmt::scat("Impossible to find point ", geo::get_x(point.coords), " ",
                                          geo::get_y(point.coords), " ", geo::get_z(point.coords), " in list of morphology points"));
    }
    return it->id;
}

std::size_t gmsh_abstract_file::add_segment(const gmsh_segment & s){
    gmsh_segment segment(s);
    add_point(segment.point1);
    add_point(segment.point2);
    segment.id = create_id_line_element();

    return _segments.insert(segment).first->id;
}

std::size_t gmsh_abstract_file::add_circle(const gmsh_circle &c){
    gmsh_circle my_circle(c);
    add_point(my_circle.center);
    add_point(my_circle.point1);
    add_point(my_circle.point2);
    my_circle.id = create_id_line_element();
    return _circles.insert(my_circle).first->id;
}


std::size_t gmsh_abstract_file::add_line_loop(const gmsh_line_loop &l){
    gmsh_line_loop loop(l);
    loop.id = _line_loop.size();
    return _line_loop.insert(loop).first->id;
}

std::size_t gmsh_abstract_file::add_volume(const gmsh_volume &v){
    gmsh_volume volume(v);
    volume.id = _volumes.size();
    return _volumes.insert(volume).first->id;
}


template<typename Value, typename Set>
std::vector<Value> sort_by_id(const Set & s){

    std::vector<Value> all_objects;
    all_objects.reserve(s.size());

    // reorder all points by id, for geo file lisibility
    std::copy(s.begin(), s.end(), std::back_inserter(all_objects));

    std::sort(all_objects.begin(), all_objects.end(), [](const Value & p1, const Value & p2){
        return (p1.id < p2.id);
    });

    return all_objects;
}


std::vector<gmsh_point> gmsh_abstract_file::get_all_points() const{
    return sort_by_id<gmsh_point>(_points);
}

std::vector<gmsh_segment> gmsh_abstract_file::get_all_segments() const{
    return sort_by_id<gmsh_segment>(_segments);
}

std::vector<gmsh_circle> gmsh_abstract_file::get_all_circles() const{
    return sort_by_id<gmsh_circle>(_circles);
}


std::vector<gmsh_line_loop> gmsh_abstract_file::get_all_line_loops() const{
    return sort_by_id<gmsh_line_loop>(_line_loop);
}


std::vector<gmsh_volume> gmsh_abstract_file::get_all_volumes() const{
    return sort_by_id<gmsh_volume>(_volumes);
}

// if point around 0 -> 0
double clean_coordinate(double val){
    if(hadoken::math::almost_equal(val, 0.0)){
        return 0;
    }
    return val;
}

void gmsh_abstract_file::export_points_to_stream(ostream &out){
    out << "\n";
    out << "// export morphology points \n";

    auto all_points = get_all_points();

    for(auto p = all_points.begin(); p != all_points.end(); ++p){
        fmt::scat(out,
                  "Point(", p->id,") = {", clean_coordinate(geo::get_x(p->coords)),", ", clean_coordinate(geo::get_y(p->coords)), ", ", clean_coordinate(geo::get_z(p->coords)), ", ", p->diameter,"};\n");
        if(p->isPhysical){
            fmt::scat(out,
                      "Physical Point(", p->id,") = {", p->id,"};\n");
        }
    }

    out << "\n\n";
}


void gmsh_abstract_file::export_segments_to_stream(ostream &out){
    out << "\n";
    out << "// export morphology segments \n";

    auto all_segments = get_all_segments();

    for(auto p = all_segments.begin(); p != all_segments.end(); ++p){
        fmt::scat(out,
                  "Line(", p->id,") = {" , find_point(p->point1),", ", find_point(p->point2),"};\n");
        if(p->isPhysical){
            fmt::scat(out,
                      "Physical Line(", p->id,") = {", p->id,"};\n");
        }
    }

    out << "\n\n";
}


void gmsh_abstract_file::export_circle_to_stream(ostream &out){
    out << "\n";
    out << "// export morphology arc-circle \n";

    auto all_circles = get_all_circles();

    for(auto p = all_circles.begin(); p != all_circles.end(); ++p){
        fmt::scat(out,
                  "Circle(", p->id,") = {" , find_point(p->point1),", ", find_point(p->center), ", ", find_point(p->point2),"};\n");
        if(p->isPhysical){
            fmt::scat(out,
                      "Physical Line(", p->id,") = {", p->id,"};\n");
        }
    }

    out << "\n\n";
}


void gmsh_abstract_file::export_line_loop_to_stream(ostream &out){
    out << "\n";
    out << "// export line_looop \n";

    auto all_loop = get_all_line_loops();

    for(auto p = all_loop.begin(); p != all_loop.end(); ++p){
      //  std::cout << " size line loop " << p->ids.size() << std::endl;
        fmt::scat(out, "Line Loop(", p->id,") = {");
        std::string delimiter = "";
        for(auto & id_lines : p->ids){
            fmt::scat(out, delimiter,id_lines);
            delimiter.assign(", ");
        }
        fmt::scat(out, "};\n");
        if(p->isRuled){
            fmt::scat(out,
                      "Ruled Surface(", p->id,") = {", p->id,"};\n");
        }
        if(p->isPhysical){
            fmt::scat(out,
                      "Physical Surface(", p->id,") = {", p->id,"};\n");
        }
    }

    out << "\n\n";
}

void gmsh_abstract_file::export_volume_to_stream(ostream &out){
    out << "\n";
    out << "// export volumes \n";

    auto all_volumes = get_all_volumes();

    // export 1 surface loop == 1 volume
    for(auto p = all_volumes.begin(); p != all_volumes.end(); ++p){
        fmt::scat(out, "Surface Loop(", p->id,") = {");
        std::string delimiter = "";
        for(auto & id: p->ids){
            fmt::scat(out, delimiter,id);
            delimiter.assign(", ");
        }
        fmt::scat(out, "};\n");
        fmt::scat(out,
                  "Volume(", p->id,") = {", p->id,"};\n");
        if(p->isPhysical){
            fmt::scat(out,
                  "Physical Volume(", p->id,") = {", p->id,"};\n");
        }
    }

    out << "\n\n";
}


// Line id are in common for circle and segment, and need consequently to
// be mutualized
std::size_t gmsh_abstract_file::create_id_line_element(){
    return _segments.size() + _circles.size();
}

gmsh_exporter::gmsh_exporter(const std::string & morphology_filename, const std::string & mesh_filename, exporter_flags my_flags) :
    geo_stream(mesh_filename),
    reader(morphology_filename),
    flags(my_flags)
{


}


void gmsh_exporter::export_to_point_cloud(){
    serialize_header();
    serialize_points_raw();
}


void gmsh_exporter::export_to_wireframe(){
    serialize_header();

    morpho_tree tree = reader.create_morpho_tree();

    gmsh_abstract_file vfile;
    construct_gmsh_vfile_lines(tree, tree.get_branch(0), vfile);

    vfile.export_points_to_stream(geo_stream);
    vfile.export_segments_to_stream(geo_stream);
}


void gmsh_exporter::export_to_3d_object(){
    serialize_header();

    morpho_tree tree = reader.create_morpho_tree();

    gmsh_abstract_file vfile;
    construct_gmsh_3d_object(tree, tree.get_branch(0), vfile);

    vfile.export_points_to_stream(geo_stream);
    vfile.export_segments_to_stream(geo_stream);
    vfile.export_circle_to_stream(geo_stream);
    vfile.export_line_loop_to_stream(geo_stream);
    vfile.export_volume_to_stream(geo_stream);
}


void gmsh_exporter::serialize_header(){
    geo_stream << gmsh_header << "\n";


    fmt::scat(geo_stream,
              gmsh_header,
              "// converted to GEO format from ", reader.get_filename(), "\n");

}


void gmsh_exporter::construct_gmsh_vfile_raw(gmsh_abstract_file & vfile){

    auto points = reader.get_points_raw();

    assert(points.size2() > 3);

    for(std::size_t row =0; row < points.size1(); ++row){
        gmsh_point point(geo::point3d( points(row, 0), points(row, 1), points(row, 2)), points(row, 3));
        vfile.add_point(point);
    }

}

void gmsh_exporter::construct_gmsh_vfile_lines(morpho_tree & tree, branch & current_branch, gmsh_abstract_file & vfile){

    const auto linestring = current_branch.get_linestring();
    if(linestring.size() > 1 && !(current_branch.get_type() == branch_type::soma && (flags & exporter_single_soma))){
        for(std::size_t i =0; i < (linestring.size()-1); ++i){
            gmsh_point p1(linestring[i], 1.0);
            p1.setPhysical(true);
            gmsh_point p2(linestring[i+1], 1.0);
            p2.setPhysical(true);

            gmsh_segment segment(p1, p2);
            segment.setPhysical(true);
            vfile.add_segment(segment);
        }
    }

    const auto & childrens = current_branch.get_childrens();
    for( auto & c : childrens){
        branch & child_branch = tree.get_branch(c);
        construct_gmsh_vfile_lines(tree, child_branch, vfile);
    }
}



void create_gmsh_sphere(gmsh_abstract_file & vfile, const geo::sphere3d & sphere){
    // create 6 points of the sphere
    std::array<gmsh_point, 2> xpoints, ypoints, zpoints;
    xpoints[0] =  gmsh_point(sphere.get_center() - geo::point3d(sphere.get_radius(), 0 ,0 ));
    xpoints[1]  = gmsh_point(sphere.get_center() + geo::point3d(sphere.get_radius(), 0 ,0 ));
    ypoints[0]  = gmsh_point(sphere.get_center() - geo::point3d(0, sphere.get_radius() ,0 ));
    ypoints[1]  = gmsh_point(sphere.get_center() + geo::point3d(0, sphere.get_radius() ,0 ));
    zpoints[0] =  gmsh_point(sphere.get_center() - geo::point3d(0, 0, sphere.get_radius() ));
    zpoints[1]  = gmsh_point(sphere.get_center() + geo::point3d(0, 0, sphere.get_radius() ));

    // We need to cut the sphere in 12 arc circle to modelize it
    // and connect the arc together with line loops
    std::vector<std::size_t> line_loops;
    for(auto & x : xpoints){
        x.setPhysical(true);
        for(auto & y : ypoints){
            y.setPhysical(true);
            gmsh_point center(sphere.get_center());

            gmsh_circle xy_circle (center, x, y);
            xy_circle.setPhysical(true);
            std::size_t xy_circle_id= vfile.add_circle(xy_circle);

            for(auto & z : zpoints){
                z.setPhysical(true);
                gmsh_point center(sphere.get_center());

                gmsh_circle xz_circle(center, x, z);
                xz_circle.setPhysical(true);
                std::size_t xz_circle_id = vfile.add_circle(xz_circle);

                gmsh_circle yz_circle(center, y, z);
                yz_circle.setPhysical(true);
                std::size_t yz_circle_id = vfile.add_circle(yz_circle);

                gmsh_line_loop line_loop({ int64_t(xy_circle_id), int64_t(yz_circle_id), int64_t(-1* xz_circle_id) });
                line_loop.setPhysical(true);
                line_loop.setRuled(true);
                line_loops.push_back(vfile.add_line_loop(line_loop));
            }
        }

    }

    vfile.add_volume(gmsh_volume(line_loops));
}


void gmsh_exporter::construct_gmsh_3d_object(morpho_tree & tree, branch & current_branch, gmsh_abstract_file & vfile){

    if(current_branch.get_type() == branch_type::soma){
        branch_soma::sphere soma_sphere = static_cast<branch_soma&>(current_branch).get_sphere();
        create_gmsh_sphere(vfile, soma_sphere);
    }else{

        auto & distance = current_branch.get_distances();

        for(std::size_t i =0; i < distance.size(); ++i){
                    create_gmsh_sphere(vfile, geo::sphere3d(current_branch.get_point(i), distance(i)));
        }


    }

    const auto linestring = current_branch.get_linestring();
    if(linestring.size() > 1 && !(current_branch.get_type() == branch_type::soma && (flags & exporter_single_soma))){
        for(std::size_t i =0; i < (linestring.size()-1); ++i){
            gmsh_point p1(linestring[i], 1.0);
            p1.setPhysical(true);
            gmsh_point p2(linestring[i+1], 1.0);
            p2.setPhysical(true);

            gmsh_segment segment(p1, p2);
            segment.setPhysical(true);
            vfile.add_segment(segment);
        }
    }

    const auto & childrens = current_branch.get_childrens();
    for( auto & c : childrens){
        branch & child_branch = tree.get_branch(c);
        construct_gmsh_3d_object(tree, child_branch, vfile);
    }
}



void gmsh_exporter::serialize_points_raw(){
    gmsh_abstract_file vfile;
    construct_gmsh_vfile_raw(vfile);
    vfile.export_points_to_stream(geo_stream);
}



} // morpho
