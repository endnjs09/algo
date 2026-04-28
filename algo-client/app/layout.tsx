import type { Metadata } from "next";
import { Inter, Fira_Code } from "next/font/google";
import "./globals.css";

const inter = Inter({
  variable: "--font-inter",
  subsets: ["latin"],
});

const firaCode = Fira_Code({
  variable: "--font-fira-code",
  subsets: ["latin"],
});

export const metadata: Metadata = {
  title: "AlGo - Algorithm Visualizer",
  description: "AI-powered C++ algorithm step-by-step visualizer",
};

import { VisualizerProvider } from "@/lib/VisualizerContext";

export default function RootLayout({
  children,
}: Readonly<{
  children: React.ReactNode;
}>) {
  return (
    <html
      lang="en"
      className={`${inter.variable} ${firaCode.variable} h-full antialiased font-sans`}
    >
      <body className="min-h-full flex flex-col bg-background text-textMain">
        <VisualizerProvider>
          {children}
        </VisualizerProvider>
      </body>
    </html>
  );
}
