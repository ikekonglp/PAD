## Goal

Dependency parsers are fast, accurate, and produce easy-to-interpret results, but phrase-structure parses are nice too and are required input for many NLP tasks.

The PAD parser produces phrases-after-dependencies. Give it the output of a dependency parser and it will produce the optimal constrained phrase-structure parse.

## Installation

```
cd src
make
```

## How to Use

```
> ./dep_parser sents.txt | ./pad -m pad.model | head
(TOP  (SINV  (CC But)   (S  (NP  (PRP you) ) )   (MD ca)   (NP  (RB n't) )   (VP  (VB dismiss)   (S  (NP  (NP  (NP  (NNP Mr.) 
   (NNP Stoltzman)   (POS 's) )   (NN music) )   (CC or)   (NP  (PRP$ his)   (NNS motives) ) ) )   (PP  (RB as)   (ADJP  (RB m 
erely)   (JJ commercial)   (CC and)   (JJ lightweight) ) ) )   (. .) ) )                                          
```

or

```
./pad --model model --sentences test.predicted.conll
```


```
>./pad --help

PAD: Phrases After Dependencies
  USAGE: pad [options]

  Options:
  --help:              Print this message and exit.
  --model, -m:         (Required) Model file.
  --sentences, -g:     CoNLL sentence file.
  --oracle, -o:        Run in oracle mode.
  --pruning, -p:        .
  --dir_pruning:        .
```

## How to Train

To train a new model, you'll need a grammar file and gold annotations. The file formats are described below. 

```
> ./padt --grammar rules --model model --annotations parts --conll train.conll --epochs 5 --simple_features
```

PADt takes the following options.

```
> ./padt --help

PADt: Phrases After Dependencies trainer
USAGE: padt [options]

Options:
--help:             Print this message and exit.
--grammar, -g:      (Required) Grammar file.
--conll, -c:        (Required) CoNLL sentence file.
--model, -m:        (Required) Model file to output.
--annotations, -a   (Required) Gold phrase structure file.
--epochs[=10], -e:  Number of epochs.
--lambda[=0.0001]:  L1 Regularization constant.
--simple_features   Use simple set of features.
```


We also provide python scripts for extracting a grammar and annotations from phrase-structure trees using the Collins head rules. 

Please refers to python/README.md

## Cite

```
@InProceedings{kong-15,
  author    = {Lingpeng Kong and Alexander M. Rush and Noah A. Smith},
  title     = {Transforming Dependencies into Phrase Structures},
  booktitle = {Proceedings of the 2015 Conference of the North American Chapter of the Association for Computational Linguistics: Human Language Technologies},
  month     = jun,
  year      = {2015},
  address   = {Denver, Colorado, USA},
  publisher = {Association for Computational Linguistics},
  sbooktitle = {NAACL-HLT~2015}
}

```

## File Formats

The grammar file has two types of lines. For unary rules:

```
RULE# 0 X Y 0
```

For binary rules:

```
RULE# 1 X Y Z HEAD
```

The annotation file is only required for training. Each line is of the form:

```
#RULES
i j k h m r
```

Where i, j, k are the span of the rule, h is the head index, m is the modifier index, and r in the index of the rule from the grammar file. 

There is no line break between sentences.
