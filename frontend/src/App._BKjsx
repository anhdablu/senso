import { useState } from 'react';

// Sound- und Positions-Konfiguration für die originalen Quadranten
const COLORS = [
  { id: 'blue',  on: 'RoyalBlue',  off: 'DarkBlue',  hz: 493, radius: '100% 0 0 0' },
  { id: 'green', on: 'LightGreen', off: 'DarkGreen', hz: 440, radius: '0 100% 0 0' },
  { id: 'red',   on: 'Red',        off: 'DarkRed',   hz: 391, radius: '0 0 0 100%' },
  { id: 'yellow',on: 'Yellow',     off: 'GoldenRod', hz: 349, radius: '0 0 100% 0' }
];

export default function App() {
  const [sequence, setSequence] = useState([]);
  const [userSequence, setUserSequence] = useState([]);
  const [activeColor, setActiveColor] = useState(null);
  const [isGameOver, setIsGameOver] = useState(false);
  const [isPlayingSequence, setIsPlayingSequence] = useState(false);

  const playSound = (hz) => {
    const ctx = new (window.AudioContext || window.webkitAudioContext)();
    const osc = ctx.createOscillator();
    osc.type = 'triangle';
    osc.frequency.value = hz;
    osc.connect(ctx.destination);
    osc.start();
    osc.stop(ctx.currentTime + 0.3);
  };

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

  const playSequence = async (seq) => {
    setIsPlayingSequence(true);
    const delay = seq.length >= 10 ? 300 : seq.length >= 5 ? 400 : 500;

    for (let color of seq) {
      setActiveColor(color.id);
      playSound(color.hz);
      await new Promise((r) => setTimeout(r, delay));
      setActiveColor(null);
      await new Promise((r) => setTimeout(r, 150));
    }
    setIsPlayingSequence(false);
  };

  const handleColorClick = (color) => {
    if (isPlayingSequence || isGameOver || sequence.length === 0) return;

    playSound(color.hz);
    setActiveColor(color.id);
    setTimeout(() => setActiveColor(null), 200);

    const nextUserSeq = [...userSequence, color];
    setUserSequence(nextUserSeq);

    const currentStep = nextUserSeq.length - 1;
    if (color.id !== sequence[currentStep].id) {
      setIsGameOver(true);
      return;
    }

    if (nextUserSeq.length === sequence.length) {
      setTimeout(() => startNextLevel(sequence), 1200);
    }
  };

  return (
    <div style={{ textAlign: 'center', fontFamily: 'sans-serif', marginTop: '40px', color: '#fff' }}>
      <h1>React Senso (Simon)</h1>
      <p style={{ fontSize: '20px' }}>Score: {sequence.length > 0 ? sequence.length - 1 : 0}</p>

      {/* Das runde Gehäuse des Originalgeräts */}
      <div style={{
        position: 'relative',
        width: '320px',
        height: '320px',
        margin: '30px auto',
        backgroundColor: '#222', // Schwarzer Rand außen/innen wie beim Original
        borderRadius: '50%',
        padding: '15px',
        boxShadow: '0 8px 24px rgba(0,0,0,0.5)',
        boxSizing: 'border-box'
      }}>
        
        {/* Spielfeld-Grid für die 4 Quadranten */}
        <div style={{ 
          display: 'grid', 
          gridTemplateColumns: '1fr 1fr', 
          width: '100%', 
          height: '100%', 
          gap: '12px'
        }}>
          {COLORS.map((color) => (
            <button
              key={color.id}
              onClick={() => handleColorClick(color)}
              style={{
                backgroundColor: activeColor === color.id ? color.on : color.off,
                border: 'none',
                // Trick: Jeder Button bekommt abgerundete Ecken passend zu seiner Position
                borderRadius: color.radius, 
                cursor: isPlayingSequence ? 'not-allowed' : 'pointer',
                transition: 'background-color 0.1s ease, transform 0.05s ease',
                boxShadow: activeColor === color.id ? `0 0 25px ${color.on}` : 'none',
                transform: activeColor === color.id ? 'scale(0.98)' : 'scale(1)'
              }}
            />
          ))}
        </div>

        {/* Die klassische schwarze Steuereinheit im Zentrum */}
        <div style={{
          position: 'absolute',
          top: '50%',
          left: '50%',
          transform: 'translate(-50%, -50%)',
          width: '110px',
          height: '110px',
          backgroundColor: '#151515',
          borderRadius: '50%',
          border: '8px solid #222',
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'center',
          boxShadow: 'inset 0 2px 5px rgba(0,0,0,0.5)'
        }}>
          {sequence.length === 0 && (
            <button onClick={startGame} style={{ 
              padding: '6px 12px', 
              fontSize: '13px', 
              fontWeight: 'bold',
              cursor: 'pointer', 
              borderRadius: '20px',
              backgroundColor: '#e74c3c',
              color: '#fff',
              border: 'none',
              boxShadow: '0 2px 4px rgba(0,0,0,0.3)'
            }}>
              START
            </button>
          )}
          {sequence.length > 0 && !isGameOver && (
            <span style={{ fontSize: '24px', fontWeight: 'bold', color: '#bdc3c7' }}>
              {sequence.length}
            </span>
          )}
          {isGameOver && (
            <button onClick={startGame} style={{ 
              padding: '6px 10px', 
              fontSize: '11px', 
              fontWeight: 'bold',
              cursor: 'pointer', 
              borderRadius: '20px',
              backgroundColor: '#34495e',
              color: '#fff',
              border: 'none'
            }}>
              AGAIN
            </button>
          )}
        </div>
      </div>

      {isGameOver && (
        <div style={{ marginTop: '10px' }}>
          <h2 style={{ color: '#ff4d4d', margin: '5px' }}>GAME OVER</h2>
        </div>
      )}
    </div>
  );
}
