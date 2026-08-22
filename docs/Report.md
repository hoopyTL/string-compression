# Data Structures & Algorithms (CSC10004) - Summer 2026
## Final Project Report: Lossless String Compression

**Department of Knowledge Engineering — Faculty of Information Technology**  
**University of Science, VNU-HCM (HCMUS)**

---

## 3.1 Group Information

| Student Name | Student ID | Assigned Tasks | % Completed |
| :--- | :---: | :--- | :---: |
| **[Student Name 1]** | `[ID 1]` | RLE Algorithm Research, Architecture Design & CLI Implementation | 100% |
| **[Student Name 2]** | `[ID 2]` | Huffman Coding Research, Min-Heap Tree Implementation & Bit-Packing | 100% |
| **[Student Name 3]** | `[ID 3]` | LZW Algorithm Research, Dynamic Dictionary Optimization & Benchmarking | 100% |
| **[Student Name 4]** | `[ID 4]` | Experimental Data Gathering, Analysis, Report Drafting & Video Demo | 100% |
| **Total Group Completion** | | | **100%** |

---

## 3.2 Problem Statement

### 1. Formal Definition
Lossless data compression is the process of encoding an input sequence of symbols $S = \langle s_1, s_2, \dots, s_N \rangle$ over a finite alphabet $\Sigma$ into a shorter binary representation $C = \langle b_1, b_2, \dots, b_M \rangle$ ($b_i \in \{0, 1\}$) such that:
1. **Size Reduction**: The total bit length $|C| < |S| \times \log_2 |\Sigma|$ for data exhibiting non-random statistical regularities (redundancy/entropy deficiency).
2. **Reversibility (Lossless Invariant)**: There exists a deterministic decoding function $\mathcal{D}$ such that $\mathcal{D}(C) \equiv S$ with zero information loss.

The efficiency of compression is governed by **Claude Shannon's Source Coding Theorem** (1948), which establishes the fundamental theoretical limit: no lossless compression scheme can compress an input source to fewer bits per symbol than the empirical entropy $H(S)$:
$$H(S) = -\sum_{x \in \Sigma} P(x) \log_2 P(x)$$
where $P(x)$ is the probability of symbol $x$ occurring in sequence $S$.

### 2. Standard Running Example
Throughout Section 3.3, we demonstrate and trace the internal operational mechanics of each algorithm using the fixed running example:
$$\mathbf{S = \text{"BABBACAC"}}$$
- Length: $N = 8$ characters (64 bits under standard 8-bit ASCII).
- Alphabet: $\Sigma = \{ \text{'A'}, \text{'B'}, \text{'C'} \}$ ($K = 3$).
- Frequencies: $f(\text{'A'}) = 3$, $f(\text{'B'}) = 3$, $f(\text{'C'}) = 2$.
- Empirical Entropy:
  $$H(S) = -\left( \frac{3}{8}\log_2\frac{3}{8} + \frac{3}{8}\log_2\frac{3}{8} + \frac{2}{8}\log_2\frac{2}{8} \right) \approx 1.561 \text{ bits/symbol}$$
- Theoretical Minimal Representation: $8 \times 1.561 \approx 12.49 \text{ bits} \approx 2 \text{ bytes}$.

---

## 3.3 Algorithm Analysis

```
                                  +-----------------------------+
                                  | Lossless Compression Family |
                                  +--------------+--------------+
                                                 |
         +---------------------------------------+---------------------------------------+
         |                                       |                                       |
+--------+--------+                     +--------+--------+                     +--------+--------+
|  Run-Length     |                     | Huffman Coding  |                     | Lempel-Ziv-Welch|
| Encoding (RLE)  |                     | (Prefix Codes)  |                     |  (LZW - Dict)   |
+-----------------+                     +-----------------+                     +-----------------+
| Exploits runs   |                     | Exploits symbol |                     | Exploits multi- |
| of identical    |                     | frequency       |                     | character       |
| adjacent symbols|                     | distribution    |                     | recurring phras.|
+-----------------+                     +-----------------+                     +-----------------+
```

---

### 3.3.1 Run-Length Encoding (RLE)

#### A. Introduction & Applications
Run-Length Encoding (RLE) is one of the simplest forms of data compression. It replaces consecutive identical symbols (called a *run*) with a single data value and a count. RLE is particularly effective on data containing long runs of repeated elements.
- **Real-World Applications**:
  - ITU-T T.4 and T.6 standard **Fax transmissions** (compressing large horizontal runs of white/black pixels).
  - Bitmap image formats such as **BMP** and **PCX** for palletized graphics.
  - Early television signal compression and intermediate stages in JPEG (for encoding sequences of zero DCT coefficients).

#### B. Language-Agnostic Pseudocode

```
Algorithm 1: RLE-Compress(Input)
Input : Sequence of bytes Input of length N
Output: Compressed byte sequence Output
1: Output <- empty byte array
2: i <- 0
3: while i < N do
4:     symbol <- Input[i]
5:     count <- 1
6:     while i + count < N and Input[i + count] == symbol and count < 255 do
7:         count <- count + 1
8:     end while
9:     Append (count, symbol) to Output
10:    i <- i + count
11: end while
12: return Output
```

```
Algorithm 2: RLE-Decompress(Compressed)
Input : Compressed byte sequence of length 2M
Output: Original byte sequence Output
1: Output <- empty byte array
2: for i <- 0 to length(Compressed) - 1 step 2 do
3:     count <- Compressed[i]
4:     symbol <- Compressed[i + 1]
5:     for j <- 1 to count do
6:         Append symbol to Output
7:     end for
8: end for
9: return Output
```

#### C. Step-by-Step Trace on Running Example: `"BABBACAC"`

| Step | Current Position | Substring Processed | Run Character | Run Length (Count) | Output Stream `(Count, Char)` |
| :---: | :---: | :---: | :---: | :---: | :--- |
| **1** | Index 0 | `"B"` | `'B'` | 1 | `(1, 'B')` |
| **2** | Index 1 | `"A"` | `'A'` | 1 | `(1, 'B'), (1, 'A')` |
| **3** | Index 2..3 | `"BB"` | `'B'` | 2 | `(1, 'B'), (1, 'A'), (2, 'B')` |
| **4** | Index 4 | `"A"` | `'A'` | 1 | `(1, 'B'), (1, 'A'), (2, 'B'), (1, 'A')` |
| **5** | Index 5 | `"C"` | `'C'` | 1 | `(1, 'B'), (1, 'A'), (2, 'B'), (1, 'A'), (1, 'C')` |
| **6** | Index 6 | `"A"` | `'A'` | 1 | `(1, 'B'), (1, 'A'), (2, 'B'), (1, 'A'), (1, 'C'), (1, 'A')` |
| **7** | Index 7 | `"C"` | `'C'` | 1 | `(1, 'B'), (1, 'A'), (2, 'B'), (1, 'A'), (1, 'C'), (1, 'A'), (1, 'C')` |

- **Output Representation**: `1B 1A 2B 1A 1C 1A 1C`
- **Output Size**: 7 pairs $\times$ 2 bytes = **14 bytes**.
- **Compression Ratio**: $\frac{8 \text{ bytes}}{14 \text{ bytes}} = 0.57\times$ (**Negative Compression / Size Expansion** due to lack of long runs).

#### D. Computational Complexity
- **Time Complexity**:
  - **Compression**: $\mathcal{O}(N)$. The pointer $i$ advances by $count$ at each step, inspecting each input byte exactly once or twice.
  - **Decompression**: $\mathcal{O}(N)$. It iterates through $\frac{M}{2}$ pairs and emits $N$ total output bytes.
- **Space Complexity**:
  - $\mathcal{O}(1)$ auxiliary memory (excluding input and output buffers), requiring only simple loop counters and single-byte variables.

#### E. Critical Boundary Discussion
- **Best-Case Scenario**: An input consisting of a single repeating symbol (e.g., $N$ bytes of `'A'`). It compresses into $\lceil N / 255 \rceil \times 2$ bytes, achieving a maximal compression ratio of $\approx 127.5\times$ and $99.2\%$ space savings.
- **Worst-Case Scenario (Negative Compression)**: Alternating non-repeating data (e.g., `"ABABABAB"`). Every run has length 1, requiring 2 bytes per symbol. The compressed output expands to exactly $2N$ bytes (a $100\%$ size inflation, ratio $0.5\times$).

---

### 3.3.2 Huffman Coding

#### A. Introduction & Applications
Invented by David Huffman (1952), Huffman Coding is an optimal entropy-based prefix coding algorithm. It assigns variable-length bit codes to input symbols such that more frequent characters receive shorter bit sequences, while rarer characters receive longer codes. Crucially, no code word is a prefix of any other code word (**Prefix Property**), enabling unambiguous streaming decoding without delimiters.
- **Real-World Applications**:
  - **DEFLATE** compression algorithm (used in **GZIP**, **PNG**, and **ZIP**).
  - Entropy coding backend in multimedia codecs like **JPEG**, **MP3**, and **AAC**.
  - **HTTP/2** header compression (HPACK).

#### B. Language-Agnostic Pseudocode

```
Algorithm 3: Huffman-Build-Tree(Frequencies, Alphabet K)
Input : Frequency array Freq of size K
Output: Root node of the optimal prefix tree
1: PriorityQueue Q <- empty min-priority queue
2: for each symbol c in Alphabet with Freq[c] > 0 do
3:     node <- CreateNode(symbol=c, freq=Freq[c], left=NULL, right=NULL)
4:     Q.Insert(node)
5: end for
6: while Q.Size() > 1 do
7:     leftNode <- Q.ExtractMin()
8:     rightNode <- Q.ExtractMin()
9:     parent <- CreateNode(symbol=NULL, freq=leftNode.freq + rightNode.freq, left=leftNode, right=rightNode)
10:    Q.Insert(parent)
11: end while
12: return Q.ExtractMin()
```

```
Algorithm 4: Huffman-Compress(Input, N)
Input : Sequence of bytes Input of length N
Output: Header metadata + Packed bitstream Output
1: Freq <- CountFrequencies(Input)
2: Root <- Huffman-Build-Tree(Freq)
3: CodeTable <- GenerateBitCodes(Root, currentBitString="")
4: BitStream <- empty bit buffer
5: Append original size N (8 bytes) and Freq table to Output
6: for each byte b in Input do
7:     BitStream.Append(CodeTable[b])
8: end for
9: Append BitStream.PackBytes() to Output
10: return Output
```

```
Algorithm 5: Huffman-Decompress(Compressed)
Input : Compressed byte stream
Output: Original byte sequence Output
1: (N, Freq, BitStream) <- ParseHeader(Compressed)
2: Root <- Huffman-Build-Tree(Freq)
3: CurrentNode <- Root
4: Output <- empty byte array
5: for each bit in BitStream while length(Output) < N do
6:     if bit == 0 then CurrentNode <- CurrentNode.left
7:     else CurrentNode <- CurrentNode.right
8:     if IsLeaf(CurrentNode) then
9:         Append CurrentNode.symbol to Output
10:        CurrentNode <- Root
11:    end if
12: end for
13: return Output
```

#### C. Step-by-Step Trace on Running Example: `"BABBACAC"`

##### Step 1: Frequency Analysis
- Symbol `'A'`: Frequency = 3
- Symbol `'B'`: Frequency = 3
- Symbol `'C'`: Frequency = 2

##### Step 2: Min-Heap Transitions & Tree Construction
1. **Initial Min-Heap State**:
   $$Q = [ (\text{'C'}: 2), (\text{'A'}: 3), (\text{'B'}: 3) ]$$
2. **Merge 1**:
   - Extract smallest two nodes: $(\text{'C'}: 2)$ and $(\text{'A'}: 3)$.
   - Create parent node $N_1$ with frequency $2 + 3 = 5$.
   - $N_1.\text{left} = \text{'C'}$, $N_1.\text{right} = \text{'A'}$.
   - Re-insert $N_1$ into heap:
     $$Q = [ (\text{'B'}: 3), (N_1: 5) ]$$
3. **Merge 2**:
   - Extract smallest two nodes: $(\text{'B'}: 3)$ and $(N_1: 5)$.
   - Create parent root node $R$ with frequency $3 + 5 = 8$.
   - $R.\text{left} = \text{'B'}$, $R.\text{right} = N_1$.
   - Re-insert $R$:
     $$Q = [ (R: 8) ] \implies \text{Tree Complete!}$$

##### Step 3: Resulting Binary Tree & Prefix Code Assignments
```
              [Root: 8]
             /         \
          0 /           \ 1
           /             \
       ('B': 3)       [N1: 5]
                     /       \
                  0 /         \ 1
                   /           \
               ('C': 2)     ('A': 3)
```

| Symbol | Frequency | Path from Root (0 = Left, 1 = Right) | Huffman Code | Bit Length |
| :---: | :---: | :---: | :---: | :---: |
| `'B'` | 3 | `0` | **`0`** | 1 bit |
| `'C'` | 2 | `1` $\to$ `0` | **`10`** | 2 bits |
| `'A'` | 3 | `1` $\to$ `1` | **`11`** | 2 bits |

##### Step 4: Bitstream Encoding of `"BABBACAC"`
| Symbol Index | Character | Encoded Code | Bitstream So Far |
| :---: | :---: | :---: | :--- |
| 0 | `'B'` | `0` | `0` |
| 1 | `'A'` | `11` | `011` |
| 2 | `'B'` | `0` | `0110` |
| 3 | `'B'` | `0` | `01100` |
| 4 | `'A'` | `11` | `0110011` |
| 5 | `'C'` | `10` | `011001110` |
| 6 | `'A'` | `11` | `01100111011` |
| 7 | `'C'` | `10` | `0110011101110` |

- **Total Payload Length**: $1 + 2 + 1 + 1 + 2 + 2 + 2 + 2 = \mathbf{13 \text{ bits}}$ (down from 64 bits!).
- **Average Code Length**: $\bar{L} = \frac{13}{8} = 1.625 \text{ bits/symbol}$ (very close to entropy $H(S) \approx 1.561$).
- **Packed Payload**: `01100111 01110000_2` = `0x67 0x70` (2 bytes payload).

#### D. Computational Complexity
- **Time Complexity**:
  - **Frequency Counting**: $\mathcal{O}(N)$ over input size $N$.
  - **Tree Construction**: $\mathcal{O}(K \log K)$ where $K = |\Sigma| \le 256$ is alphabet size.
  - **Encoding / Decoding**: $\mathcal{O}(N)$ to traverse string and bitstream.
  - **Total Time**: $\mathcal{O}(N + K \log K) \equiv \mathcal{O}(N)$ since $K$ is bounded by 256 for byte streams.
- **Space Complexity**:
  - Tree storage: $\mathcal{O}(K)$ nodes (at most $2K - 1 \le 511$ nodes).
  - Code mapping table: $\mathcal{O}(K)$ entries.
  - Total auxiliary space: $\mathcal{O}(K) = \mathcal{O}(1)$ for fixed byte alphabet.

#### E. Critical Boundary Discussion
- **Best-Case Scenario**: Heavily skewed probability distribution (e.g., $99\%$ occurrences of one symbol). Huffman assigns a 1-bit code to the dominant symbol, achieving an encoding size close to $\frac{N}{8}$ bytes (theoretical $8\times$ compression).
- **Worst-Case Scenario**: Uniform distribution over all 256 byte values ($P(x) = \frac{1}{256}$). Every symbol gets an 8-bit code, yielding zero compression. When including the canonical frequency table header (1032 bytes in standard binary format), files smaller than $\approx 1 \text{ KB}$ experience minor negative compression.

---

### 3.3.3 Lempel-Ziv-Welch (LZW)

#### A. Introduction & Applications
Published by Terry Welch (1984) as an improvement over LZ78, LZW is a dictionary-based lossless compression algorithm. Unlike Huffman, LZW does not require passing statistical frequency models or tree headers; instead, it dynamically constructs an adaptive vocabulary table during both compression and decompression entirely based on previously observed symbol sequences.
- **Real-World Applications**:
  - Classic UNIX `compress` utility.
  - **GIF** (Graphics Interchange Format) image compression.
  - **TIFF** image format and **PostScript / PDF** document streams.

#### B. Language-Agnostic Pseudocode

```
Algorithm 6: LZW-Compress(Input, MaxDictSize)
Input : Sequence of bytes Input of length N
Output: Sequence of integer codes Output
1: Dictionary <- Initialize with single-byte entries (0..255 -> 0..255)
2: nextCode <- 256
3: currentString <- ""
4: for each byte c in Input do
5:     combined <- currentString + c
6:     if combined is in Dictionary then
7:         currentString <- combined
8:     else
9:         Output.Append(Dictionary[currentString])
10:        if nextCode < MaxDictSize then
11:            Dictionary[combined] <- nextCode
12:            nextCode <- nextCode + 1
13:        end if
14:        currentString <- "" + c
15:    end if
16: end for
17: if currentString != "" then
18:    Output.Append(Dictionary[currentString])
19: end if
20: return Output
```

```
Algorithm 7: LZW-Decompress(CodeStream, MaxDictSize)
Input : Sequence of integer codes CodeStream
Output: Original byte sequence Output
1: Dictionary <- Initialize with single-byte entries (0..255 -> strings 0..255)
2: nextCode <- 256
3: previousCode <- CodeStream[0]
4: Output.Append(Dictionary[previousCode])
5: for i <- 1 to length(CodeStream) - 1 do
6:     currentCode <- CodeStream[i]
7:     if currentCode is in Dictionary then
8:         entry <- Dictionary[currentCode]
9:     else if currentCode == nextCode then
10:        entry <- Dictionary[previousCode] + Dictionary[previousCode][0]
11:    end if
12:    Output.Append(entry)
13:    if nextCode < MaxDictSize then
14:        Dictionary[nextCode] <- Dictionary[previousCode] + entry[0]
15:        nextCode <- nextCode + 1
16:    end if
17:    previousCode <- currentCode
18: end for
19: return Output
```

#### C. Step-by-Step Trace on Running Example: `"BABBACAC"`

##### Initial Dictionary Configuration:
- Codes `0..255`: Individual ASCII byte values (`'A' = 65`, `'B' = 66`, `'C' = 67`, ...).
- Next available dynamic dictionary index: `nextCode = 256`.

##### Compression Trace Table:

| Step | Current Prefix $w$ | Next Char $c$ | Concatenation $w + c$ | In Dictionary? | Emitted Code | Action / New Dictionary Entry |
| :---: | :---: | :---: | :---: | :---: | :---: | :--- |
| **1** | `""` | `'B'` | `"B"` | Yes (code 66) | *(none)* | Set $w \leftarrow \text{"B"}$ |
| **2** | `"B"` | `'A'` | `"BA"` | **No** | **66** (`'B'`) | Dict[256] = `"BA"`, Set $w \leftarrow \text{"A"}$ |
| **3** | `"A"` | `'B'` | `"AB"` | **No** | **65** (`'A'`) | Dict[257] = `"AB"`, Set $w \leftarrow \text{"B"}$ |
| **4** | `"B"` | `'B'` | `"BB"` | **No** | **66** (`'B'`) | Dict[258] = `"BB"`, Set $w \leftarrow \text{"B"}$ |
| **5** | `"B"` | `'A'` | `"BA"` | **Yes** (code 256) | *(none)* | Set $w \leftarrow \text{"BA"}$ |
| **6** | `"BA"` | `'C'` | `"BAC"` | **No** | **256** (`"BA"`) | Dict[259] = `"BAC"`, Set $w \leftarrow \text{"C"}$ |
| **7** | `"C"` | `'A'` | `"CA"` | **No** | **67** (`'C'`) | Dict[260] = `"CA"`, Set $w \leftarrow \text{"A"}$ |
| **8** | `"A"` | `'C'` | `"AC"` | **No** | **65** (`'A'`) | Dict[261] = `"AC"`, Set $w \leftarrow \text{"C"}$ |
| **End**| `"C"` | `EOF` | — | — | **67** (`'C'`) | Emit final code for $w$ |

- **Emitted Code Stream**: `[66, 65, 66, 256, 67, 65, 67]` (7 codes total).
- **Encoded Size (using 16-bit / 2-byte codes)**: $7 \times 2 \text{ bytes} = \mathbf{14 \text{ bytes}}$.

#### D. Computational Complexity
- **Time Complexity**:
  - **Compression**: $\mathcal{O}(N)$ using hash maps or trie dictionary lookups ($\mathcal{O}(1)$ average lookup time per input character).
  - **Decompression**: $\mathcal{O}(N)$ array index lookups to retrieve string phrases.
- **Space Complexity**:
  - $\mathcal{O}(D)$ where $D$ is the maximum dictionary size (bounded by `MAX_DICT_SIZE = 65536` entries in standard 16-bit implementations).

#### E. Critical Boundary Discussion
- **Best-Case Scenario**: Long repetitive phrases with structured vocabulary redundancy (e.g., repeating `"ABCDABCD..."`). The phrase length grows quadratically with emitted codes, yielding immense compression ratios ($\ge 70\times$).
- **Worst-Case Scenario & Initial Overhead**:
  - Short files or non-repeating data (e.g., random bytes). Because each code word is emitted as 16 bits (2 bytes) to accommodate the expanding index table, emitting 1 code for 1 character actually doubles the size (200% inflation). LZW requires sufficient repetition and large file sizes ($> 50 \text{ KB}$) to amortize this 16-bit code overhead.

---

## 3.4 Experimental Results

### 1. Benchmark Measurements Table (Format of Table 1)

The table below reports experimental results obtained from running `run_experiments.sh` across Scenario 1 (varying file size) and Scenario 2 (varying data entropy). All values are averaged across multiple test runs.

| File Size | Data Type | RLE Time (Ratio) | Huffman Time (Ratio) | LZW Time (Ratio) |
| :---: | :---: | :---: | :---: | :---: |
| **10 KB** | Standard English | 0.00 ms (0.56x) | 0.33 ms (1.50x) | 5.00 ms (1.34x) |
| **100 KB** | Standard English | 2.00 ms (0.54x) | 4.00 ms (1.70x) | 50.00 ms (2.04x) |
| **500 KB** | Standard English | 11.33 ms (0.52x) | 21.00 ms (1.73x) | 233.67 ms (2.57x) |
| **1 MB** (1024 KB) | Standard English | 23.67 ms (0.51x) | 41.33 ms (1.74x) | 442.67 ms (2.60x) |
| **5 MB** (5120 KB) | Standard English | 112.00 ms (0.51x) | 203.33 ms (1.72x) | 2138.67 ms (2.47x) |
| **10 MB** (10240 KB)| Standard English | 222.33 ms (0.51x) | 402.33 ms (1.72x) | 4155.67 ms (2.45x) |
| **1 MB** (1024 KB) | Highly Repetitive | 6.00 ms (2.50x) | 18.00 ms (3.32x) | 352.67 ms (72.83x) |
| **1 MB** (1024 KB) | Completely Random | 23.00 ms (0.50x) | 64.33 ms (1.00x) | 666.67 ms (0.80x) |

---

### 2. Comprehensive Performance Analysis & Discussion

#### Question 1: Does the growth rate in your data match the Big-O complexity analyzed in Section 3.3?
- **Analysis**:
  - For **RLE**, as the file size scaled from $100\text{ KB} \to 1\text{ MB} \to 10\text{ MB}$ ($10\times$ and $100\times$ increase), the execution time scaled from $2.00\text{ ms} \to 23.67\text{ ms} \to 222.33\text{ ms}$, exhibiting a strictly linear $\mathcal{O}(N)$ trajectory with negligible constant factor overhead.
  - For **Huffman Coding**, execution time scaled from $4.00\text{ ms} \to 41.33\text{ ms} \to 402.33\text{ ms}$. This perfectly validates the theoretical $\mathcal{O}(N + K \log K)$ bound, where the $K \log K$ tree building phase is constant for $K = 256$, rendering the operational time linear $\mathcal{O}(N)$ with the input stream length.
  - For **LZW**, execution time scaled from $50.00\text{ ms} \to 442.67\text{ ms} \to 4155.67\text{ ms}$. The growth is near-linear $\mathcal{O}(N)$ with a higher constant factor attributed to dictionary hash table insertions and dynamic memory allocations.

#### Question 2: Which algorithm achieved the highest compression ratio on standard text versus highly repetitive data?
- **Standard English Text**:
  - **LZW** achieved the highest compression ratio (**$2.60\times$**, saving $61.5\%$ space) once the file size reached $\ge 500\text{ KB}$, outperforming Huffman Coding (**$1.74\times$**, saving $42.4\%$). LZW excels here because natural English prose possesses multi-character repetitive patterns (e.g., prefixes `"th"`, `"ing"`, words `"the "`, `"which "`), allowing LZW to encode multi-byte words into single 16-bit codes.
  - RLE failed completely on English text ($0.51\times$, negative compression) because consecutive identical characters are rare in English prose.
- **Highly Repetitive Data**:
  - **LZW** achieved an overwhelming compression ratio of **$72.83\times$** ($98.63\%$ space savings), vastly outperforming Huffman ($3.32\times$) and RLE ($2.50\times$). LZW rapidly builds multi-kilobyte dictionary phrases from repetitive sequences, drastically compressing thousands of repeated characters into isolated 2-byte indices.

#### Question 3: Explain why these results happened based on the theoretical limits of each algorithm.
- **Shannon Entropy Limits**:
  - In **Random Data** ($H(S) \approx 8.0 \text{ bits/symbol}$), no statistical pattern or frequency skew exists. Huffman strictly converges to $1.00\times$ ratio (bounded by Shannon's theorem), while RLE ($0.50\times$) and LZW ($0.80\times$) suffer from code expansion overhead.
  - In **Repetitive Data**, the entropy $H(S) \to 0$. Huffman is fundamentally constrained to assign at least $1 \text{ bit}$ per symbol, capping its compression ratio at $\le 8\times$. Conversely, dictionary-based algorithms like **LZW** bypass the 1-bit-per-character limit by encoding variable-length substrings of length $L \gg 8$, enabling near-infinite theoretical compression ratios ($72.83\times$).

---

## 3.5 References

Following the **IEEE Citation Style**:

1. T. H. Cormen, C. E. Leiserson, R. L. Rivest, and C. Stein, "Greedy Algorithms: Huffman Codes," in *Introduction to Algorithms*, 3rd ed. Cambridge, MA, USA: MIT Press, 2009, ch. 16, sec. 3, pp. 428–437.
2. D. A. Huffman, "A Method for the Construction of Minimum-Redundancy Codes," *Proceedings of the IRE*, vol. 40, no. 9, pp. 1098–1101, Sept. 1952, doi: 10.1109/JRPROC.1952.273898.
3. T. A. Welch, "A Technique for High-Performance Data Compression," *Computer*, vol. 17, no. 6, pp. 8–19, June 1984, doi: 10.1109/MC.1984.1659158.
4. C. E. Shannon, "A Mathematical Theory of Communication," *The Bell System Technical Journal*, vol. 27, no. 3, pp. 379–423, July 1948, doi: 10.1002/j.1538-7305.1948.tb01338.x.
5. J. Ziv and A. Lempel, "A Universal Algorithm for Sequential Data Compression," *IEEE Transactions on Information Theory*, vol. 23, no. 3, pp. 337–343, May 1977, doi: 10.1109/TIT.1977.1055714.

---

## 3.6 Acknowledgment

We would like to express our gratitude to our instructors, **Assoc. Prof. Dr. Le Thanh Tung** and **Dr. Tran Hoang Quan** (Department of Knowledge Engineering, FIT - HCMUS), for providing comprehensive guidelines and academic insights throughout the Data Structures & Algorithms course. In addition, generative AI tools (Google Gemini / Antigravity IDE) assisted with verifying edge-case tests and formatting benchmark performance logs into structured analytical tables.
