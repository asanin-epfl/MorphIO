import morphotool
from morphotool import NEURON_STRUCT_TYPE
from os import path
import re

# loader: by default morpho_h5_v1
_h5loader = morphotool.MorphoReader

class Morphology(_h5loader, object):
    """
    A class representing a Morphology.
    It builds on a Morphology Reader, but will keep a Morpho_Tree instance if any of these methods are used.
    This avoids creating the tree if everything the user does is getting raw data
    """

    # morpho_tree cache. Default is None (avoid AttributeError)
    _morpho_tree = None

    def __init__(self, morpho_dir, morpho_name, layer=None, mtype=None):
        super(Morphology, self).__init__(path.join(morpho_dir, morpho_name+".h5"))
        self._name_attrs = MorphNameExtract(morpho_name)
        self.layer = layer
        self.mtype = mtype

    def __repr__(self):
        return "<%s Morphology>" % (self.label,)

    def __getattr__(self, item):
        if hasattr(morphotool.MorphoTree, item):
            if self._morpho_tree is None:
                self._morpho_tree = self.create_morpho_tree()
            attr = getattr(self._morpho_tree, item)
            # print attr
            # if hasattr(attr, '__get__'):
            #     attr = attr.__get__(self, Morphology)
            # print attr
            return attr
        raise AttributeError(item)

    @property
    def morpho_tree(self):
        if self._morpho_tree is None:
            self._morpho_tree = self.create_morpho_tree()
        return self._morpho_tree

    @property
    def name_attrs(self):
        return self._name_attrs

    @property
    def label(self):
        return self._name_attrs.name

    @property
    def type(self):
        return None

    def get_section(self, section_id):
        return self.get_node(section_id)

    @property
    def soma(self):
        #s = self.morpho_tree.get_soma()
        #return Soma(s, self._morpho_tree)
        return self.get_soma()

    @property
    def neurites(self):
        return self.axon + self.dendrites

    @property
    def axon(self):
        return self.find_nodes(NEURON_STRUCT_TYPE.axon)

    @property
    def dendrites(self):
        return self.basal_dendrites + self.apical_dendrites

    @property
    def basal_dendrites(self):
        return self.find_nodes(NEURON_STRUCT_TYPE.dentrite_basal)

    @property
    def apical_dendrites(self):
        return self.find_nodes(NEURON_STRUCT_TYPE.dentrite_apical)

    @property
    def sections(self):
        return self.get_all_nodes()

    @property
    def root(self):
        return self.get_node(0)

    @property
    def path_to_soma(self):
        return None

    @property
    def mesh(self):
        return None

    # @property
    # def bounding_box(self):
    #     return self.get_bounding_box()

    def __str__(self):
        pass


# class Section(object):
#
#     def __new__(cls, branch_obj, tree):
#         branch_obj.__class__= cls
#         return branch_obj
#
#     def __init__(self, branch_object, tree):    # type: (morphotool.NeuronBranch, morphotool.MorphoTree) -> None
#         self._morpho_tree = tree                # type: morphotool.MorphoTree
#
#     @property
#     def children(self):
#         return None
#
#     @property
#     def compartments(self):
#         return None
#
#     @property
#     def compartment(self):
#         return None
#
#     @property
#     def cross_section(self):
#         return None
#
#     @property
#     def cross_sections(self):
#         return None
#
#     @property
#     def num_compartments(self):
#         return None
#
#     @property
#     def graft(self):
#         return None
#
#     @property
#     def has_parent(self):
#         return None
#
#     @property
#     def has_compartments(self):
#         return None
#
#     @property
#     def is_ancestor_of(self):
#         return None
#
#     @property
#     def is_descendent_of(self):
#         return None
#
#     @property
#     def length(self):
#         return self.get_number_points()
#
#     @property
#     def morphology(self):
#         return None
#
#     def move_point(self):
#         pass
#
#     @property
#     def parent(self):
#         parent_id = self.branch_obj.get_parent()
#         if parent_id == 0:
#             return None
#         return Section(self.morpho_tree.get_branch(self.branch_obj.get_parent()))
#
#     @property
#     def path_to_soma(self):
#         return None
#
#     @property
#     def type(self):
#         return None
#
#     def section_distance(self, segment_id, middle_point=True):
#         pass
#
#     @property
#     def segments(self):
#         return None
#
#     def get_segment(self, id):
#         pass
#
#     def split_segment(self):
#         pass
#
#     def resize_diameter(self):
#         pass
#
#     def __str__(self):
#         pass
#
#
# class Soma(Section,):
#     def position(self):
#         pass
#
#     def mean_radius(self):
#         pass
#
#     def max_radius(self):
#         pass
#
#     def surface_points(self):
#         pass
#
#     def __str__(self):
#         pass



class MorphologyDB(object):
    @staticmethod
    def split_line_to_details(line):
        parts = line.split()
        return parts[0], (int(parts[1]), parts[2])

    def __init__(self, db_path, db_file=None):
        self.db_path = db_path
        self._db = {}
        self._morphos_layer_mtype = {}
        if db_file:
            with open(path.join(db_path, db_file)) as f:
                self._morphos_layer_mtype = { self.split_line_to_details(l) for l in f }


    def __getitem__(self, morpho_name):
        item = self._db.get(morpho_name)
        if not item:
            item = self._db[morpho_name] = Morphology(self.db_path, morpho_name, *self._morphos_layer_mtype.get(item,()))
        return item



# -----------------------------------------------------------
# Aux functions
# -----------------------------------------------------------
class MorphNameExtract(object):
    scaled_mixed_morphname_pattern = re.compile(
        r"dend\-(?P<dend>[\w\-\+]+)_axon\-(?P<axon>[\w\-\+]+)_\-"
        r"_Scale_x(?P<scale_x>[\d]+.[\d]+)_y(?P<scale_y>[\d]+.[\d]+)_z(?P<scale_z>[\d]+.[\d]+)"
    )
    scaled_morphname_pattern = re.compile(
        r"(?P<name>[\w\-\+]+)_\-_Scale_x(?P<scale_x>[\d]+.[\d]+)"
        r"_y(?P<scale_y>[\d]+.[\d]+)_z(?P<scale_z>[\d]+.[\d]+)"
    )
    mixed_morphname_pattern = re.compile(
        r"dend\-(?P<dend>[\w\-\+]+)_axon\-(?P<axon>[\w\-\+]+)"
    )

    def __init__(self, name, mixed=None, scaled=None, cloned=None):
        mixed =False
        cloned=False
        scaled=False

        """ Extraction results """
        if mixed is None:
            self.morphname_extract(name)
        else:
            self.name = name
            self.mixed = mixed
            self.scaled = scaled
            self.cloned = cloned

    def morphname_extract(self, name):
        """ Try and extract the morphname """
        self.cloned = "_-_Clone_" in name
        self.scaled = "_-_Scale_" in name
        self.mixed = "dend-" in name

        if self.cloned:
            name = name[0:str.find(name, '_-_Clone_')]  # Remove from name

        m=None
        attributes=None
        if self.mixed and self.scaled:
            m = self.scaled_mixed_morphname_pattern.match(name)
            attributes = ("dend", "axon", "scale_x", "scale_y", "scale_z")
            name = name[0:str.find(name, '_-_Scale_')]
        elif self.scaled and not self.mixed:
            m = self.scaled_morphname_pattern.match(name)
            attributes = ('name', 'scale_x', 'scale_y', 'scale_z')
        elif self.mixed and not self.scaled:
            m = self.mixed_morphname_pattern.match(name)
            attributes = ('dend', 'axon')

        self.name = name

        if not m:
            raise ValueError("misformatted morph name %s" % name)

        for a in attributes:
            setattr(self, a, m.group(a))

