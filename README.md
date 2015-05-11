## Goal

Dependency parsers are fast, accurate, and produce easy-to-interpret results, but phrase-structure parses are nice too and are required input for many NLP tasks.

The PAD parser produces phrases-after-dependencies. Give it the output of a dependency parser and it will produce the optimal constrained phrase-structure parse.

## How to Use.

```
> ./dep_parser sents.txt | ./pad -m pad.model | head
(TOP  (SINV  (CC But)   (S  (NP  (PRP you) ) )   (MD ca)   (NP  (RB n't) )   (VP  (VB dismiss)   (S  (NP  (NP  (NP  (NNP Mr.) 
   (NNP Stoltzman)   (POS 's) )   (NN music) )   (CC or)   (NP  (PRP$ his)   (NNS motives) ) ) )   (PP  (RB as)   (ADJP  (RB m 
erely)   (JJ commercial)   (CC and)   (JJ lightweight) ) ) )   (. .) ) )                                          
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

To train a new model, you'll need a lexicalized grammar file and gold annotations. The file formats are described below. 

```
> ./padt --conll sents.conll --annotations sents.ptb --grammar ptb.rules -m pad.model
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
--simple_features   Use simple set of features (debugging).
```


We also provide python scripts for extracting a grammar and annotations from phrase-structure trees using the Collins head rules. 


TODO: Add docs here.

## Cite


```

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
