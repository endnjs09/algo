import React from 'react';
import Link from 'next/link';
import { Code2 } from 'lucide-react';

export function Header() {
  return (
    <header className="fixed top-0 left-0 right-0 z-50 flex items-center justify-between px-8 py-3 backdrop-blur-md bg-[#0f172a]/60 border-b border-gray-800/50 transition-all">
      <Link href="/" className="flex items-center gap-2 group transition-all">
        <div className="w-9 h-9 rounded-xl bg-white text-black flex items-center justify-center font-black text-xl shadow-lg group-hover:scale-105 transition-transform">
          A
        </div>
        <span className="text-xl font-bold tracking-tighter text-white/90">
          Algo
        </span>
      </Link>
      
      <div className="flex items-center gap-4">
        <Link 
          href="https://github.com/endnjs09/algo"
          target="_blank"
          className="flex items-center gap-2 px-4 py-2 text-sm font-semibold rounded-full bg-white/5 border border-white/10 text-white/80 hover:bg-white/10 hover:text-white transition-all"
        >
          <Code2 size={18} />
          GitHub
        </Link>
      </div>
    </header>
  );
}
