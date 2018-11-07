# Gofer2 - GSEA as a service

## Table of Contents
* [Compiling Gopher2 from source](https://github.com/bschiffthaler/gofer2#compiling-gopher2-from-source)
  * [Requirements](https://github.com/bschiffthaler/gofer2#requirements)
  * [Building](https://github.com/bschiffthaler/gofer2#building)
* [The config file](https://github.com/bschiffthaler/gofer2#the-config-file)
* [File formats](https://github.com/bschiffthaler/gofer2#file-formats)
  * [Mappings](https://github.com/bschiffthaler/gofer2#mappings)
  * [Annotations](https://github.com/bschiffthaler/gofer2#annotations)
* [The Endpoints](https://github.com/bschiffthaler/gofer2#the-endpoints)
  * [The enrichment endpoint](https://github.com/bschiffthaler/gofer2#the-enrichment-endpoint)
  * [The gene-to-term endpoint](https://github.com/bschiffthaler/gofer2#the-gene-to-term-endpoint)
  * [The term-to-gene endpoint](https://github.com/bschiffthaler/gofer2#the-term-to-gene-endpoint)

## Compiling Gopher2 from source

### Requirements:
* CMake (>3.0)
* Boost (>1.60)
* CppRestSDK (>2.10.1)

The basic recipe assumes that you have dependencies in standard locations:

```bash
git clone https://github.com/bschiffthaler/gofer2
mkdir build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

If you have custom locations for the dependencies, you can specify them with environment variables:
* BOOST_ROOT
* OPENSSL_ROOT
* CPPREST_ROOT

```bash
git clone https://github.com/bschiffthaler/gofer2
mkdir build
BOOST_ROOT=/usr/local/my_location OPENSSL_ROOT=/usr/local/my_location CPPREST=/usr/local/my_location cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

## The config file

`Gofer2` requires a single argument: a JSON formatted config file. This file has the following layout:

```json
{
    "port": 5432,
    "org":[
        {
            "name": "string",
            "uri": "string",
            "enrichment": [
                {
                    "name": "string",
                    "test": "go|hierarchical|fisher",
                    "annotation": "string",
                    "mapping": "file"
                }
            ],
            "background": [
                {
                    "name": "string",
                    "sourcefile": "file"
                }
            ]
        }
    ],
    "annotation": [
        {
            "name": "string",
            "sourcefile": "file",
            "type": "flat|hierarchical|go"
        },
        {
            "name": "GO_basic",
            "sourcefile": "/home/bs/script/go-basic.obo",
            "type": "go",
            "parent": ["part_of", "regulates"]
        }
    ]
}
```

At the root of this JSON file, there are three entries:
* **port** (Optional): The port on which the server should listen - a number. The default is `5432`
* **org** (Required): A JSON array of "organisms", i.e. species for which to host the API. Each array member has four possible fields:
  * **name** (Required): A name for this organism, e.g.: `"Populus tremula"`
  * **uri** (Required): A URL _suffix_ for this organism, e.g.: `"potra"`
  * **enrichment** (Required): A JSON array with an entry for each enrichment target that should be served for this organism. There are four mandatory arguments
    * **name**: A _unique_ name for this test (only needs to be unique within the org), e.g.: `"go"`
    * **test**: One of "fisher", "go", or "hierarchical". Defines which statistical test to use for this dataset
    * **annotation**: The name of the corresponding entry in the "annotation" section of the config file
    * **mapping**: Path to a file that maps gene names to terms for this test.
  * **background** (Optional): A JSON array with gene lists for backgrounds. Has two mandatory fields:
    * **name**: The _unique_ name of this background, e.g.: "all_expressed_in_root"
    * **sourcefile**: A path to a file with genes (one per line) belonging to this background
* **annotation** (Required): Here we have annotation files, which define metadata terms. As an example, here we could put the Gene Ontology (GO) obo file, which defines the terms names, definitions and relationships. Each entry has three mandatory fields:
  * **name**: The name of this annotation. This is referred to from the enrichment section of the config.
  * **type**: The file format, "go" for a GO `.obo` file, flat for TAB separated values and hierarchical for TAB separated values where the first column describes a hierarchical relationship.
  * **sourcefile**: Path to the file containing the annotation.
  * **parent** (Only for type "go"): In this array, one can define which additional GO relationships constitute a "parent" of a term. The relationship `"is_a"`, is by default always considered. Options here are: `["part_of", "regulates", "negatively_regulates", "positively_regulates", "occurs_in"]`, or simply the string `"all"` for all relationships.

## File formats

### Mappings

File formats for gene to term mappings (the "mapping" files in the "enrichment" block of the config files) have a simple purpose: describe which terms belong to a gene ID. The format is two columns of a TAB separated file. The first column is the gene ID, the second is any number of term IDs separated by vertical pipes `|`. Once may supply one mapping per row (as shown in the first two lines), or multiple mappings in a line (as in the third line) or any combination of this.

```
Potra000799g06336       GO:0003723
Potra000799g06336       GO:0008168
Potra000798g06333       GO:0005525|GO:0005622|GO:0006886
```

### Annotations

#### GO OBO

The GO annotation is best obtained from here: http://snapshot.geneontology.org/ontology/go-basic.obo

#### Custom flat annotations

Custom annotations can be supplied as a three column TAB separated file in the form "ID    Name    Description". An example here from the PFAM annotation:

```
PF00085 Thioredoxin     Thioredoxins are small enzymes that participate in redox reactions, via the reversible oxidation of an active centre disulfide bond. Some members with only the active site are not separated from the noise.
PF00086 Thyroglobulin type-1 repeat     Thyroglobulin type 1 repeats are thought to be involved in the control of proteolytic degradation [2]. The domain usually contains six conserved cysteines.  These form three disulphide bridges.  Cysteines 1 pairs with 2, 3 with 4 and 5 with 6.
PF00087 Snake toxin and toxin-like protein      This family predominantly includes venomous neurotoxins and  cytotoxins from snakes, but also structurally similar (non-snake) toxin-like proteins (TOLIPs) such as Lymphocyte antigen 6D and  Ly6/PLAUR domain-containing protein. Snake toxins are short  proteins with a compact, disulphide-rich structure. TOLIPs have  similar structural features (abundance of spaced cysteine residues,  a high frequency of charge residues, a signal peptide for secretion  and a compact structure) but, are not associated with a venom  gland or poisonous function. They are endogenous animal proteins  that are not restricted to poisonous animals [1].
```

#### Custom hierarchical annotations

Here, one can supply a hierarchical annotation file, which has the same format as the previous flat file, but the ID column describes a hierarchy. This is often used for nested network clusters. In this example, there are two outer layers, the first of which contains two children:

```
OuterLayer1:InnerLayer1    Some Name   Some description
OuterLayer1:InnerLayer2    Some Name   Some description
OuterLayer2:InnerLayer1    Some Name   Some description
```

## The Endpoints

For each organism in the config file `{"org": [{}, ...]}`, `Gofer2` will create several endpoints. As an example, imagine this config file:

```json
{
    "port": 5432,
    "org":[
        {
            "name": "Populus tremula",
            "uri": "potra",
            "enrichment": [
                {
                    "name": "GO",
                    "test": "go",
                    "annotation": "GO_basic",
                    "mapping": "/home/bs/gofer2/gene_to_go.tsv"
                },
                {
                    "name": "PFAM",
                    "test": "fisher",
                    "annotation": "PFAM",
                    "mapping": "/home/bs/gofer2/gene_to_pfam.tsv"
                }
            ]
        }
    ],
    "annotation": [
        {
            "name": "PFAM",
            "sourcefile": "/home/bs/gofer2/pfam_annotation.tsv",
            "type": "flat"
        },
        {
            "name": "GO_basic",
            "sourcefile": "/home/bs/gofer2/go-basic.obo",
            "type": "go",
            "parent": ["part_of", "regulates"]
        }
    ]
}
```

In this example, we are serving one organism (_Populus tremula_) and two enrichment tests (GO and PFAM). After executing, `Gofer2` will create the the following endpoints:

* `localhost:5432/potra/enrichment`
* `localhost:5432/potra/gene-to-term`
* `localhost:5432/potra/term-to-gene`

We can now send POST requests to these endpoints.

### The enrichment endpoint

This endpoint, served under `.../enrichment` responds to enrichment requests. Here is an example:

```json
{
  "target":["GO","PFAM"], 
  "genes": [ 
    "Potra000167g00627", "Potra000488g03037", "Potra000807g06409", 
    "Potra001021g08534", "Potra001230g10582", "Potra002409g18324", 
    "Potra002707g19806", "Potra002846g20119", "Potra002888g20235", 
    "Potra002914g20296", "Potra003868g23243", "Potra003935g23615", 
    "Potra004051g24387"], 
  "include_defs": true,
  "include_names": true,
  "alpha": 0.05
}
```

The fields are:
* **target** (Required): A JSON array with each element referring to a test in the "enrichment" section of the config file that should be carried out.
* **genes** (Required): A JSON array of genes that constitute the test set.
* **include_defs** (Optional): Wether the response should include definitions of the terms (from the annotation file)
* **include_name** (Optional): Wether the response should include names of the terms (from the annotation file)
* **alpha** (Optional): Only return terms with adjusted P values less than this value
* **background** (Optional): Either a string that matches a background defined in the config file, or a list of genes to create a background _ad hoc_ (the latter is slow, the former is always preferred if possible)

The server responds with:

```json
{
  "go": [
    {
      "def": "Catalysis of the transfer of a glycosyl group from one compound (donor) to another (acceptor). [GOC:jl, ISBN:0198506732]",
      "id": "GO:0016757",
      "mpat": 3336,
      "mt": 511,
      "name": "transferase activity, transferring glycosyl groups",
      "namespace": "MF",
      "npat": 13,
      "nt": 13,
      "padj": 4.934965817768405e-10,
      "pval": 2.2431662808038206e-11
    },
    {
      "def": "Catalysis of the transfer of a group, e.g. a methyl group, glycosyl group, acyl group, phosphorus-containing, or other groups, from one compound (generally regarded as the donor) to another compound (generally regarded as the acceptor). Transferase is the systematic name for any enzyme of EC class 2. [ISBN:0198506732]",
      "id": "GO:0016740",
      "mpat": 8740,
      "mt": 3336,
      "name": "transferase activity",
      "namespace": "MF",
      "npat": 13,
      "nt": 13,
      "padj": 2.2612844045000308e-05,
      "pval": 3.597497916250049e-06
    },
    {
      "def": "Interacting selectively and non-covalently with any protein or protein complex (a complex of two or more proteins that may include other nonprotein molecules). [GOC:go_curators]",
      "id": "GO:0005515",
      "mpat": 12715,
      "mt": 5064,
      "name": "protein binding",
      "namespace": "MF",
      "npat": 8,
      "nt": 8,
      "padj": 0.002313364942911223,
      "pval": 0.0006309177117030609
    }
  ],
  "pfam": [
    {
      "def": "This RING/U-box type zinc-binding domain is frequently found in the catalytic subunit (irx3) of cellulose synthase. The enzymic class is EC:2.4.1.12, whereby the synthase removes the glucose from UDP-glucose and adds it to the growing cellulose, thereby releasing UDP. The domain-structure is treble-clef like (PDB:1weo).",
      "id": "PF14569",
      "m": 25597,
      "mt": 15,
      "n": 13,
      "name": "Zinc-binding RING-finger",
      "nt": 12,
      "padj": 7.180023120723292e-41,
      "pval": 3.590011560361646e-41
    },
    {
      "def": "Cellulose, an aggregate of unbranched polymers of beta-1,4-linked glucose residues, is the major component of wood and thus paper, and is synthesised by plants, most algae, some bacteria and fungi, and even some animals. The genes that synthesise cellulose in higher plants differ greatly from the well-characterised genes found in Acetobacter and Agrobacterium sp. More correctly designated as cellulose synthase catalytic subunits, plant cellulose synthase (CesA) proteins are integral membrane proteins, approximately 1,000 amino acids in length. There are a number of highly conserved residues, including several motifs shown to be necessary for processive glycosyltransferase activity [1].",
      "id": "PF03552",
      "m": 25597,
      "mt": 35,
      "n": 13,
      "name": "Cellulose synthase",
      "nt": 13,
      "padj": 4.553357794278433e-39,
      "pval": 4.553357794278433e-39
    }
  ]
}
```

In the response, there will be a root term for each test we asked for ("go" and "pfam") and in there, we will have an array, with one object per term. In the term we can have the following fields:

* **id**: The term's ID
* **def** (if `"include_defs": true`): The definition string of this term
* **name** (if `"include_names": true`): The name of this term
* **m** (Only "flat"): The number of genes in the population. The number of genes in the whole population or in the background with at least one annotation to a term.
* **n** (Only "flat"): The number of genes in the test set with at least one annotation to a term.
* **mt**: The number of genes in the entire population annotated to this term
* **nt**: The number of genes in the test set annotated to this term
* **mpat** (Only "go" and "hierarchical"): The number of genes annotated to this term or _parents_ of it in the entire population
* **npat** (Only "go" and "hierarchical"): The number of genes annotated to this term or _parents_ of it in the test set
* **pval**: The naive p-value
* **padj**: The p-value adjusted for multiple testing
* **namespace** (only "go"): The gene ontology namespace. Either MF for "Molecular function", BP for "Biological process", or CC for "Cellular Component"

### The gene-to-term endpoint

This is a convenience endpoint that allows the user to look up mapping information. An example request looks like this:

```json
{
  "target":["go","pfam","kegg"], 
  "genes": ["Potra000167g00627", "Potra000488g03037"], 
  "include_defs": true,
  "include_names": true
}
```

The fields are:
* **target**: A JSON array of mappings (from the "enrichment" sections of the config) for which the terms belonging to the genes should be returned.
* **genes**: A JSON array of genes for which the mappings should be returned
* **include_defs** (Optional): Wether to include the terms definitions.
* **include_names** (Optional): Wether to include the terms names.

The server responds with:

```json
{
  "go": [
    {
      "id": "Potra000488g03037",
      "terms": [
        {
          "def": "Interacting selectively and non-covalently with any protein or protein complex (a complex of two or more proteins that may include other nonprotein molecules). [GOC:go_curators]",
          "id": "GO:0005515",
          "name": "protein binding",
          "namespace": "MF"
        },
        {
          "def": "Interacting selectively and non-covalently with zinc (Zn) ions. [GOC:ai]",
          "id": "GO:0008270",
          "name": "zinc ion binding",
          "namespace": "MF"
        },
        {
          "def": "A lipid bilayer along with all the proteins and protein complexes embedded in it an attached to it. [GOC:dos, GOC:mah, ISBN:0815316194]",
          "id": "GO:0016020",
          "name": "membrane",
          "namespace": "CC"
        },
        {
          "def": "Catalysis of the reaction: UDP-glucose + ((1,4)-beta-D-glucosyl)(n) = UDP + ((1,4)-beta-D-glucosyl)(n+1). [EC:2.4.1.12]",
          "id": "GO:0016760",
          "name": "cellulose synthase (UDP-forming) activity",
          "namespace": "MF"
        },
        {
          "def": "The chemical reactions and pathways resulting in the formation of cellulose, a linear beta1-4 glucan of molecular mass 50-400 kDa with the pyranose units in the -4C1 conformation. [GOC:mah, ISBN:0198506732]",
          "id": "GO:0030244",
          "name": "cellulose biosynthetic process",
          "namespace": "BP"
        }
      ]
    },
    {
      "id": "Potra000167g00627",
      "terms": [
        {
          "def": "Interacting selectively and non-covalently with any protein or protein complex (a complex of two or more proteins that may include other nonprotein molecules). [GOC:go_curators]",
          "id": "GO:0005515",
          "name": "protein binding",
          "namespace": "MF"
        },
        {
          "def": "Interacting selectively and non-covalently with zinc (Zn) ions. [GOC:ai]",
          "id": "GO:0008270",
          "name": "zinc ion binding",
          "namespace": "MF"
        },
        {
          "def": "A lipid bilayer along with all the proteins and protein complexes embedded in it an attached to it. [GOC:dos, GOC:mah, ISBN:0815316194]",
          "id": "GO:0016020",
          "name": "membrane",
          "namespace": "CC"
        },
        {
          "def": "Catalysis of the reaction: UDP-glucose + ((1,4)-beta-D-glucosyl)(n) = UDP + ((1,4)-beta-D-glucosyl)(n+1). [EC:2.4.1.12]",
          "id": "GO:0016760",
          "name": "cellulose synthase (UDP-forming) activity",
          "namespace": "MF"
        },
        {
          "def": "The chemical reactions and pathways resulting in the formation of cellulose, a linear beta1-4 glucan of molecular mass 50-400 kDa with the pyranose units in the -4C1 conformation. [GOC:mah, ISBN:0198506732]",
          "id": "GO:0030244",
          "name": "cellulose biosynthetic process",
          "namespace": "BP"
        }
      ]
    }
  ],
  "pfam": [
    {
      "id": "Potra000488g03037",
      "terms": [
        {
          "def": "Cellulose, an aggregate of unbranched polymers of beta-1,4-linked glucose residues, is the major component of wood and thus paper, and is synthesised by plants, most algae, some bacteria and fungi, and even some animals. The genes that synthesise cellulose in higher plants differ greatly from the well-characterised genes found in Acetobacter and Agrobacterium sp. More correctly designated as cellulose synthase catalytic subunits, plant cellulose synthase (CesA) proteins are integral membrane proteins, approximately 1,000 amino acids in length. There are a number of highly conserved residues, including several motifs shown to be necessary for processive glycosyltransferase activity [1].",
          "id": "PF03552",
          "name": "Cellulose synthase"
        },
        {
          "def": "This RING/U-box type zinc-binding domain is frequently found in the catalytic subunit (irx3) of cellulose synthase. The enzymic class is EC:2.4.1.12, whereby the synthase removes the glucose from UDP-glucose and adds it to the growing cellulose, thereby releasing UDP. The domain-structure is treble-clef like (PDB:1weo).",
          "id": "PF14569",
          "name": "Zinc-binding RING-finger"
        }
      ]
    },
    {
      "id": "Potra000167g00627",
      "terms": [
        {
          "def": "Cellulose, an aggregate of unbranched polymers of beta-1,4-linked glucose residues, is the major component of wood and thus paper, and is synthesised by plants, most algae, some bacteria and fungi, and even some animals. The genes that synthesise cellulose in higher plants differ greatly from the well-characterised genes found in Acetobacter and Agrobacterium sp. More correctly designated as cellulose synthase catalytic subunits, plant cellulose synthase (CesA) proteins are integral membrane proteins, approximately 1,000 amino acids in length. There are a number of highly conserved residues, including several motifs shown to be necessary for processive glycosyltransferase activity [1].",
          "id": "PF03552",
          "name": "Cellulose synthase"
        },
        {
          "def": "This RING/U-box type zinc-binding domain is frequently found in the catalytic subunit (irx3) of cellulose synthase. The enzymic class is EC:2.4.1.12, whereby the synthase removes the glucose from UDP-glucose and adds it to the growing cellulose, thereby releasing UDP. The domain-structure is treble-clef like (PDB:1weo).",
          "id": "PF14569",
          "name": "Zinc-binding RING-finger"
        }
      ]
    }
  ]
}
```

The response has again one root term for each translation we requested ("go" and "pfam"):

* **root**: Whichever metadata was requested. In our case either "go" or "pfam". Contains a JSON array of genes
  * **id**: The gene id for the current gene
  * **terms**: A JSON array of terms applicable to the current gene
    * **id**: The ID of the term.
    * **def** (if `"include_defs": true`): The definition string of this term
    * **name** (if `"include_names": true`): The name of this term

### The term-to-gene endpoint

This is the complement to the previous endpoint. The user supplies a list of terms and the server answers with genes that are annotated to those terms. A request could look like this:

```json
{
  "target":[
    {
      "name": "go",
      "terms": ["GO:0030244", "GO:0016760"]
    },
    {
      "name": "pfam",
      "terms": ["PF03552"]
    }
  ]
}
```

The server responds with

```json
{
  "go": [
    {
      "ids": [
        "Potra000167g00627",
        "Potra000473g02869",
        "Potra000488g03037",
        "Potra000490g03079",
        "Potra000490g03080",
        "Potra000490g34919",
        "Potra000503g03273",
        "Potra000807g06409",
        "Potra000882g07128",
        "Potra001021g08534",
        "Potra001034g08699",
        "Potra001085g09396",
        "Potra001230g10582",
        "Potra001288g11102",
        "Potra001426g12051",
        "Potra001526g12689",
        "Potra001665g13679",
        "Potra002239g17201",
        "Potra002250g17293",
        "Potra002409g18324",
        "Potra002583g19426",
        "Potra002707g19806",
        "Potra002846g20119",
        "Potra002888g20235",
        "Potra002910g20286",
        "Potra002914g20296",
        "Potra003265g21167",
        "Potra003411g21584",
        "Potra003868g23243",
        "Potra003908g23460",
        "Potra003908g23461",
        "Potra003935g23615",
        "Potra004051g24387",
        "Potra004085g24472",
        "Potrs000033g00082"
      ],
      "term": "GO:0016760"
    },
    {
      "ids": [
        "Potra000167g00627",
        "Potra000473g02869",
        "Potra000488g03037",
        "Potra000490g03079",
        "Potra000490g03080",
        "Potra000490g34919",
        "Potra000503g03273",
        "Potra000683g05293",
        "Potra000807g06409",
        "Potra000882g07128",
        "Potra001021g08534",
        "Potra001034g08699",
        "Potra001085g09396",
        "Potra001230g10582",
        "Potra001247g10721",
        "Potra001288g11102",
        "Potra001426g12051",
        "Potra001526g12689",
        "Potra001665g13679",
        "Potra001830g14739",
        "Potra002139g16566",
        "Potra002239g17201",
        "Potra002250g17293",
        "Potra002409g18324",
        "Potra002583g19426",
        "Potra002596g19508",
        "Potra002707g19806",
        "Potra002846g20119",
        "Potra002888g20235",
        "Potra002910g20286",
        "Potra002914g20296",
        "Potra003265g21167",
        "Potra003411g21584",
        "Potra003861g23203",
        "Potra003868g23243",
        "Potra003908g23460",
        "Potra003908g23461",
        "Potra003935g23615",
        "Potra004051g24387",
        "Potra004085g24472",
        "Potra004401g24885",
        "Potrs000033g00082"
      ],
      "term": "GO:0030244"
    }
  ],
  "pfam": [
    {
      "ids": [
        "Potra000167g00627",
        "Potra000473g02869",
        "Potra000488g03037",
        "Potra000490g03079",
        "Potra000490g03080",
        "Potra000490g34919",
        "Potra000503g03273",
        "Potra000807g06409",
        "Potra000882g07128",
        "Potra001021g08534",
        "Potra001034g08699",
        "Potra001085g09396",
        "Potra001230g10582",
        "Potra001288g11102",
        "Potra001426g12051",
        "Potra001526g12689",
        "Potra001665g13679",
        "Potra002239g17201",
        "Potra002250g17293",
        "Potra002409g18324",
        "Potra002583g19426",
        "Potra002707g19806",
        "Potra002846g20119",
        "Potra002888g20235",
        "Potra002910g20286",
        "Potra002914g20296",
        "Potra003265g21167",
        "Potra003411g21584",
        "Potra003868g23243",
        "Potra003908g23460",
        "Potra003908g23461",
        "Potra003935g23615",
        "Potra004051g24387",
        "Potra004085g24472",
        "Potrs000033g00082"
      ],
      "term": "PF03552"
    }
  ]
}
```

At the root, we have again the annotations we requested. Within those we receive an array with one entry per term:

* **root**: The current annotation, in our case either "go" or "pfam". Contains an array of term->gene mappings
  * **term**: The ID of the current term
  * **ids**: An array of gene IDs corresponding to the current term
