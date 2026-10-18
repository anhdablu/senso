import { useState, useEffect } from 'react';

// 1. Sound-Konfiguration (Frequenzen wie im Original-Code)
const COLORS = [
  { id: 'blue',  on: 'RoyalBlue',  off: 'DarkBlue',  hz: 493 },
  { id: 'green', on: 'LightGreen', off: 'DarkGreen', hz: 440 },
  { id: 'red',   on: 'Red',        off: 'DarkRed',   hz: 391 },
  { id: 'yellow',on: 'Yellow',     off: 'GoldenRod', hz: 349 }
];

export default function SensoGame() {
  const [sequence, setSequence] = useState([]);
  const [userSequence, setUserSequence] = useState([]);
  const [activeColor, setActiveColor] = useState(null);
  const [isGameOver, setIsGameOver] = useState(false);
  const [isPlayingSequence, setIsPlayingSequence] = useState(false);

  // 2. Sound abspielen (Web Audio API)
  const playSound = (hz) => {
    const ctx = new (window.AudioContext || window.webkitAudioContext)();
    const osc = ctx.createOscillator();
    osc.type = 'triangle';
    osc.frequency.value = hz;
    osc.connect(ctx.destination);
    osc.start();
    osc.stop(ctx.currentTime + 0.3); // 300ms Tonspur
  };

  // 3. Spiel starten oder nächstes Level triggern
  const startNextLevel = (currentSeq = sequence) => {
    const randomColor = COLORS[Math.floor(Math.random() * 4)];
    const nextSeq = [...currentSeq, randomColor];
    setSequence(nextSeq);
    setUserSequence([]);
    playSequence(nextSeq);
  };

  const startGame = () => {
    setIsGameOver(false);
    startNextLevel([]);
  };

  // 4. Die Sequenz für den Spieler nacheinander aufblinken lassen
  const playSequence = async (seq) => {
    setIsPlayingSequence(true);
    // Dynamische Geschwindigkeit je nach Rundenanzahl (wie im Original)
    const delay = seq.length >= 10 ? 300 : seq.length >= 5 ? 400 : 500;

    for (let color of seq) {
      setActiveColor(color.id);
      playSound(color.hz);
      await new Promise((r) => setTimeout(r, delay));
      setActiveColor(null);
      await new Promise((r) => setTimeout(r, 150)); // Kurze Pause zwischen Tönen
    }
    setIsPlayingSequence(false);
  };

  // 5. Klick-Logik des Spielers
  const handleColorClick = (color) => {
    if (isPlayingSequence || isGameOver || sequence.length === 0) return;

    playSound(color.hz);
    const nextUserSeq = [...userSequence, color];
    setUserSequence(nextUserSeq);

    // Prüfen, ob der Klick korrekt war
    const currentStep = nextUserSeq.length - 1;
    if (color.id !== sequence[currentStep].id) {
      setIsGameOver(true);
      return;
    }

    // Runde erfolgreich beendet?
    if (nextUserSeq.length === sequence.length) {
      setTimeout(() => startNextLevel(), 1000);
    }
  };

  return (
    <div style={{ textAlign: 'center', fontFamily: 'sans-serif', marginTop: '40px' }}>
      <h1>React Senso (Simon)</h1>
      <p>Score: {sequence.length > 0 ? sequence.length - 1 : 0}</p>

      {/* Spielfeld (Einfaches CSS-Grid statt schwerem Raphael.js SVG) */}
      <div style={{ 
        display: 'grid', 
        gridTemplateColumns: '1fr 1fr', 
        width: '300px', 
        height: '300px', 
        margin: '20px auto',
        gap: '10px'
      }}>
        {COLORS.map((color) => (
          <button
            key={color.id}
            onClick={() => handleColorClick(color)}
            style={{
              backgroundColor: activeColor === color.id ? color.on : color.off,
              border: 'none',
              borderRadius: '15px',
              cursor: isPlayingSequence ? 'not-allowed' : 'pointer',
              transition: 'background-color 0.1s ease',
              height: '140px'
            }}
          />
        ))}
      </div>

      {/* Kontrollen & Overlays */}
      {sequence.length === 0 && (
        <button onClick={startGame} style={{ padding: '10px 20px', fontSize: '18px' }}>Start</button>
      )}

      {isGameOver && (
        <div>
          <h2 style={{ color: 'red' }}>GAME OVER</h2>
          <button onClick={startGame} style={{ padding: '10px 20px' }}>Neu starten</button>
        </div>
      )}
    </div>
  );
}
