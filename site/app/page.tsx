import Image from "next/image";

export default function Home() {
  return (
    <main className="relative min-h-screen w-full overflow-hidden bg-black">
      {/* Hero image — full bleed, covers the viewport */}
      <div className="relative w-full h-screen">
        <Image
          src="/vroom_car.png"
          alt="Vroom — high-performance charting"
          fill
          priority
          className="object-cover object-center"
        />

        {/* Dark vignette overlay — lightest at center where the car logo is */}
        <div
          className="absolute inset-0 z-[5]"
          style={{
            background:
              "radial-gradient(ellipse 50% 40% at 50% 45%, transparent 0%, rgba(0,0,0,0.4) 50%, rgba(0,0,0,0.85) 100%)",
          }}
        />

        {/* Text overlay — positioned over the image */}
        <div className="absolute inset-0 z-10 flex flex-col items-center justify-end pb-24">
          <h1
            className="font-black tracking-tighter text-white"
            style={{ fontSize: "clamp(3rem, 10vw, 6rem)" }}
          >
            vroom
          </h1>
          <p className="mt-4 text-lg md:text-xl text-[var(--color-subtle)] max-w-xl mx-auto text-center">
            Lightning-fast charts. Built on C++ & Skia.
          </p>

          {/* CTA */}
          <a
            href="#"
            className="inline-block mt-10 px-8 py-3 border border-[var(--color-neon-cyan)] rounded-lg text-[var(--color-neon-cyan)] font-semibold transition-all duration-300 hover:shadow-[0_0_20px_rgba(0,217,255,0.4)] hover:scale-105"
          >
            Get Started
          </a>
        </div>
      </div>
    </main>
  );
}
