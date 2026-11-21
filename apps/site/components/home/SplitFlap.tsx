'use client';

import { useEffect, useState } from 'react';

const GLYPHS = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789';

/** A word in departure-board tiles. Each tile riffles once, then lands. */
export function SplitFlap({ word, size = 'lg', still = false }: { word: string; size?: 'sm' | 'lg'; still?: boolean }) {
  const [shown, setShown] = useState(() => (still ? word : word.replace(/./g, ' ')));

  useEffect(() => {
    if (still) {
      setShown(word);
      return;
    }
    let frame = 0;
    const id = setInterval(() => {
      frame++;
      setShown(
        [...word]
          .map((ch, i) => (frame > 6 + i * 3 ? ch : GLYPHS[(frame * 7 + i * 11) % GLYPHS.length]))
          .join(''),
      );
      if (frame > 6 + word.length * 3) clearInterval(id);
    }, 55);
    return () => clearInterval(id);
  }, [word, still]);

  return (
    <span className={`flap flap--${size}`} role="img" aria-label={word}>
      {[...shown].map((ch, i) => (
        <span className="flap__tile" key={i} aria-hidden>
          {ch}
        </span>
      ))}
    </span>
  );
}
