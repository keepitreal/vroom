import type { Metadata } from "next";
import "./globals.css";

export const metadata: Metadata = {
  title: "vroom — Lightning-Fast Candlestick Charts",
  description:
    "High-performance charting library built on C++ & Skia. Native rendering for iOS, Android, React Native, and Web.",
  openGraph: {
    title: "vroom — Lightning-Fast Candlestick Charts",
    description:
      "High-performance charting library built on C++ & Skia. Native rendering for iOS, Android, React Native, and Web.",
    type: "website",
  },
};

export default function RootLayout({
  children,
}: {
  children: React.ReactNode;
}) {
  return (
    <html lang="en">
      <head>
        <link rel="preconnect" href="https://fonts.googleapis.com" />
        <link
          rel="preconnect"
          href="https://fonts.gstatic.com"
          crossOrigin="anonymous"
        />
        <link
          href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700;900&display=swap"
          rel="stylesheet"
        />
      </head>
      <body>{children}</body>
    </html>
  );
}
