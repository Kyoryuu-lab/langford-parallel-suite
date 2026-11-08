<h1>langford-parallel-suite</h1>
<p>Langford Sequences: Comprehensive evaluation of Sequential DFS vs. tbb::task_group vs. Subproblem parallel_reduce</p>

<h2>Project description</h2>
<p>This repository houses the complete suite exploring task parallelism and recursion elimination for Langford sequences:</p>
<ol>
  <li>
    <dl>
      <dt>Compares three paradigms for solving L(2, n):</dt>
      <dd><b>1. Sequential Baseline:</b> Recursive depth-first search with rollback.</dd>
      <dd><b>2. Dynamic Task Group (Variante 1):</b> Dynamic branching via <code>tbb::task_group</code> and work-stealing.</dd>
      <dd><b>3. Pre-allocated Subproblem Pool (Variante 2):</b> Unrolls recursion to a fixed tree depth, placing boards into a subproblem vector solved lock-free via <code>tbb::parallel_reduce</code>.</dd>
    </dl>
  </li>
  <li>Eliminates atomic synchronization contention entirely in Variante 2.</li>
  <li>Includes comprehensive benchmarking, speedup validation, and parallel efficiency reporting.</li>
</ol>

<h2>Project files</h2>
<table>
  <tr>
    <th colspan="2">langford-parallel-suite</th>
  </tr>
  <tr>
    <td>CMakeLists.txt</td>
    <td>Complete multi-target CMake configuration linking oneAPI TBB</td>
  </tr>
  <tr>
    <td>main.cpp</td>
    <td>Unified benchmark comparing Sequential, Task Group, and Parallel Reduce solutions</td>
  </tr>
  <tr>
    <td>README.md</td>
    <td>Comparative report and scaling analysis</td>
  </tr>
</table>

<h2>Performance Benchmark (n = 12, Board size = 24)</h2>
<table>
  <tr>
    <th>Strategy</th>
    <th>Execution Time</th>
    <th>Found Solutions</th>
    <th>Speedup</th>
    <th>Efficiency (p = 15)</th>
  </tr>
  <tr>
    <td><b>Sequential Baseline</b></td>
    <td>10,820.40 ms</td>
    <td>1,079,232</td>
    <td>1.00x</td>
    <td>100.0 %</td>
  </tr>
  <tr>
    <td><b>Variante 1 (tbb::task_group)</b></td>
    <td>1,195.62 ms</td>
    <td>1,079,232</td>
    <td>9.05x</td>
    <td>60.3 %</td>
  </tr>
  <tr>
    <td><b>Variante 2 (parallel_reduce)</b></td>
    <td>940.18 ms</td>
    <td>1,079,232</td>
    <td>11.51x</td>
    <td>76.7 %</td>
  </tr>
</table>

<h2>Installation and launch</h2>
<ol>
  <b>
    <li>
      <dl>
        <dt>Clone repository</dt>
        <dd>
          <code>git clone https://github.com/Kyoryuu-lab/langford-parallel-suite.git</code>
        </dd>
        <dd>
          <code>cd langford-parallel-suite</code>
        </dd>
      </dl>
    </li>
  </b>
  <b>
    <li>
      <dl>
        <dt>Configure and compile</dt>
        <dd>
          <code>cmake -B build -DCMAKE_BUILD_TYPE=Release</code>
        </dd>
        <dd>
          <code>cmake --build build --config Release</code>
        </dd>
      </dl>
    </li>
  </b>
  <b>
    <li>
      <dl>
        <dt>Launch full benchmark</dt>
        <dd>
          <code>./build/main</code>
        </dd>
      </dl>
    </li>
  </b>
</ol>

<h2>Example of console output</h2>
<samp>
  <p>
    ========================================================================<br />
    PVA Aufgabe 4: Langford Sequence Solver Benchmark (n = 12)<br />
    Active Threads: 15<br />
    ========================================================================<br />
    [1] Running Sequential Baseline...<br />
    Time: 10820.40 ms | Solutions: 1079232<br />
    <br />
    [2] Running Variante 1 (tbb::task_group with Cutoff)...<br />
    Time: 1195.62 ms  | Solutions: 1079232 | Speedup: 9.05x | Eff: 60.3 %<br />
    <br />
    [3] Running Variante 2 (Subproblem Pool & parallel_reduce)...<br />
    Time: 940.18 ms   | Solutions: 1079232 | Speedup: 11.51x | Eff: 76.7 %<br />
    ========================================================================<br />
    Validation: ALL RESULTS MATCH EXACTLY (SUCCESS)<br />
  </p>
</samp>
