"use client";

import React, { useRef } from "react";
import Link from "next/link";
import { Header } from "@/components/layout/Header";
import { ArrowRight, Play, CheckCircle2, Globe, Users, BookOpen, Zap } from "lucide-react";

export default function Home() {
  const guideRef = useRef<HTMLElement>(null);

  const scrollToGuide = () => {
    guideRef.current?.scrollIntoView({ behavior: "smooth" });
  };

  return (
    <div className="flex flex-col min-h-screen bg-[#0f172a] text-white selection:bg-blue-500 selection:text-white">
      {/* Mesh Gradient Background for Depth */}
      <div className="fixed inset-0 z-0 pointer-events-none overflow-hidden">
        <div className="absolute top-[-10%] left-[-10%] w-[50%] h-[50%] bg-blue-900/20 blur-[150px] rounded-full" />
        <div className="absolute bottom-[10%] right-[-5%] w-[40%] h-[40%] bg-indigo-900/20 blur-[120px] rounded-full" />
      </div>

      <Header />

      <main className="relative z-10 pt-20">
        {/* Hero Section */}
        <section className="container mx-auto px-6 py-12 md:py-16">
          <div className="flex flex-col md:flex-row items-center gap-16">
            <div className="flex-1 text-left">
              <div className="mb-4 inline-flex items-center gap-2 px-3 py-1 rounded-full bg-white/5 border border-white/10 text-[10px] font-bold tracking-widest text-blue-400 uppercase">
                <Zap size={12} fill="currentColor" />
                Algorithm Visualizer
              </div>

              <h1 className="text-4xl md:text-5xl font-black mb-6 tracking-tighter leading-[1.0]">
                Visualize code execution <br />
                <span className="text-gray-500">for C++ Algorithms</span>
              </h1>

              <p className="text-lg text-gray-400 max-w-xl mb-12 leading-relaxed">
                Understand complex logic through step-by-step interactive visualizations.
                Perfect for students, teachers, and developers to debug and learn programming visually.
              </p>

              <div className="flex items-center gap-4">
                <Link
                  href="/visualize"
                  className="flex items-center gap-2 px-8 py-4 rounded-xl bg-blue-600 text-white font-bold text-lg hover:bg-blue-500 transition-all shadow-[0_0_20px_rgba(37,99,235,0.3)] active:scale-95"
                >
                  Start Visualizing
                  <ArrowRight size={20} />
                </Link>
                <button
                  onClick={scrollToGuide}
                  className="px-8 py-4 rounded-xl bg-white/5 border border-white/10 text-white font-bold text-lg hover:bg-white/10 transition-all active:scale-95"
                >
                  How to Use
                </button>
              </div>
            </div>

            <div className="flex-1 w-full max-w-lg">
              <div className="grid grid-cols-2 gap-4">
                {[
                  { icon: Globe, label: "Various Algorithms & Data Structures", sub: "BFS, Sort, Tree..." },
                  { icon: Users, label: "Interactive Debugging", sub: "Step-by-step trace" },
                  { icon: BookOpen, label: "CS Education", sub: "Built for learners" },
                  { icon: CheckCircle2, label: "Zero Setup", sub: "Run in browser" },
                ].map((item, i) => (
                  <div key={i} className="p-8 rounded-2xl bg-white/5 border border-white/10 backdrop-blur-sm hover:border-blue-500/50 transition-all group">
                    <item.icon className="text-blue-500 mb-4 group-hover:scale-110 transition-transform" size={28} />
                    <h4 className="font-bold text-base text-white mb-2 tracking-tight">{item.label}</h4>
                    <p className="text-sm text-gray-500 leading-snug">{item.sub}</p>
                  </div>
                ))}
              </div>
            </div>
          </div>
        </section>

        {/* Visual Preview Section */}
        <section className="bg-black/20 border-y border-white/5 py-24">
          <div className="container mx-auto px-6">
            <div className="flex flex-col md:flex-row gap-16 items-center">
              <div className="flex-1 order-2 md:order-1">
                <div className="rounded-2xl overflow-hidden shadow-2xl border border-white/10 bg-[#1e1e1e]">
                  <div className="bg-black/40 px-4 py-3 border-b border-white/5 flex gap-2">
                    <div className="w-2.5 h-2.5 rounded-full bg-red-500/80" />
                    <div className="w-2.5 h-2.5 rounded-full bg-yellow-500/80" />
                    <div className="w-2.5 h-2.5 rounded-full bg-green-500/80" />
                  </div>
                  <div className="p-8 font-mono text-sm text-blue-300 leading-relaxed overflow-hidden">
                    <span className="text-purple-400">#include</span> &lt;iostream&gt; <br/>
                    <span className="text-purple-400">#include</span> &lt;vector&gt; <br/>
                    <span className="text-purple-400">using namespace</span> std; <br/><br/>
                    <span className="text-purple-400">int</span> main() &#123; <br/>
                    &nbsp;&nbsp;vector&lt;<span className="text-purple-400">int</span>&gt; arr = &#123;5, 3, 1, 4, 2&#125;; <br/>
                    &nbsp;&nbsp;<span className="text-purple-400">int</span> n = arr.size(); <br/>
                    &nbsp;&nbsp;<span className="text-purple-400">for</span> (<span className="text-purple-400">int</span> i = 0; i &lt; n - 1; i++) &#123; <br/>
                    &nbsp;&nbsp;&nbsp;&nbsp;<span className="text-purple-400">for</span> (<span className="text-purple-400">int</span> j = 0; j &lt; n - i - 1; j++) &#123; <br/>
                    &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span className="text-purple-400">if</span> (arr[j] &gt; arr[j + 1]) &#123; <br/>
                    &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;swap(arr[j], arr[j + 1]); <br/>
                    &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&#125; <br/>
                    &nbsp;&nbsp;&nbsp;&nbsp;&#125; <br/>
                    &nbsp;&nbsp;&#125; <br/>
                    &nbsp;&nbsp;<span className="text-purple-400">return</span> 0; <br/>
                    &#125;
                  </div>
                </div>
              </div>
              <div className="flex-1 order-1 md:order-2 space-y-8">
                <h2 className="text-4xl font-bold tracking-tight text-white">Learn Programming Visually</h2>
                <p className="text-gray-400 leading-relaxed text-xl">
                  Algo is the tool specifically optimized for C++ algorithm visualization.
                  Write code in the browser and see what happens at every step as the computer executes it.
                </p>
                <div className="space-y-4">
                  {['Visualize variables, objects, and pointers', 'Inspect complex graph structures in real-time', 'Perfect for interview prep and CS homework'].map((text, i) => (
                    <div key={i} className="flex items-center gap-4 text-gray-300">
                      <div className="w-5 h-5 rounded-full bg-blue-500/20 flex items-center justify-center border border-blue-500/30">
                        <CheckCircle2 size={12} className="text-blue-400" />
                      </div>
                      <span className="text-lg">{text}</span>
                    </div>
                  ))}
                </div>
              </div>
            </div>
          </div>
        </section>

        {/* Simple Guide Section */}
        <section ref={guideRef} className="container mx-auto px-6 py-32 border-t border-white/5">
          <div className="max-w-3xl mx-auto text-center mb-20">
            <h2 className="text-4xl font-bold mb-4 tracking-tight">How to Use</h2>
            <p className="text-gray-400 text-lg">Master Algo in three simple steps.</p>
          </div>

          <div className="grid grid-cols-1 md:grid-cols-3 gap-12">
            {[
              { step: "01", title: "Write Code", desc: "Enter your standard C++ algorithm code into our powerful Monaco-based editor." },
              { step: "02", title: "Analyze", desc: "Click the Visualize button. Our server analyzes the logic and generates trace events." },
              { step: "03", title: "Explore", desc: "Use the playback controls to step through execution and see data structures evolve." }
            ].map((item, i) => (
              <div key={i} className="group relative p-10 rounded-3xl bg-white/5 border border-white/5 hover:border-white/10 transition-all">
                <div className="text-4xl font-black text-blue-500/20 mb-6 group-hover:text-blue-500/40 transition-colors">
                  {item.step}
                </div>
                <h4 className="text-2xl font-bold mb-4 tracking-tight text-white">{item.title}</h4>
                <p className="text-gray-400 leading-relaxed">{item.desc}</p>
              </div>
            ))}
          </div>
        </section>
      </main>

      <footer className="py-20 border-t border-white/5 bg-black/20 text-center">
        <p className="text-gray-500 text-sm font-medium tracking-wide">© 2026 Algo Visualizer. Built for the future of algorithm education.</p>
      </footer>
    </div>
  );
}
