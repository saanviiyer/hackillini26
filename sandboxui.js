import React, { useState, useEffect } from 'react';

const SandboxTracker = () => {
    const GRID_SIZE = 5;

    // Starting at bottom-left: x=0, y=4 (assuming 0,0 is top-left in web coordinates)
    const startPos = { x: 0, y: 4 };
    const [path, setPath] = useState([startPos]);
    const [currentPos, setCurrentPos] = useState(startPos);

    // Helper to convert grid coordinates to percentage-based centers for the SVG
    const getCenter = (x, y) => ({
        cx: `${(x + 0.5) * (100 / GRID_SIZE)}%`,
        cy: `${(y + 0.5) * (100 / GRID_SIZE)}%`,
    });

    // --- HACKATHON SIMULATION CODE ---
    // This simulates the robot moving. Replace this with your Arduino/IMU WebSocket listener.
    useEffect(() => {
        const demoMoves = [
            { x: 1, y: 4 }, { x: 2, y: 4 }, { x: 2, y: 3 },
            { x: 1, y: 3 }, { x: 1, y: 4 } // Loops back over a previous spot!
        ];

        let step = 0;
        const interval = setInterval(() => {
            if (step < demoMoves.length) {
                const nextMove = demoMoves[step];
                setCurrentPos(nextMove);
                setPath(prev => [...prev, nextMove]);
                step++;
            } else {
                clearInterval(interval);
            }
        }, 1000);

        return () => clearInterval(interval);
    }, []);
    // ---------------------------------

    return (
        <div className="flex flex-col items-center p-6 bg-white rounded-xl shadow-lg font-sans">
            <h2 className="text-2xl font-bold mb-4 text-gray-800">Sand-Leveling Telemetry</h2>

            {/* The Sandbox Container */}
            <div className="relative w-full max-w-md aspect-square bg-[#fdfbf2] border-4 border-gray-300 rounded-md overflow-hidden">

                {/* 1. Base Grid (CSS Grid) */}
                <div className="absolute inset-0 grid grid-cols-5 grid-rows-5">
                    {Array.from({ length: GRID_SIZE * GRID_SIZE }).map((_, i) => (
                        <div key={i} className="border border-gray-200/80" />
                    ))}
                </div>

                {/* 2. Path Overlay (SVG) */}
                <svg className="absolute inset-0 w-full h-full pointer-events-none">
                    {path.map((pt, index) => {
                        if (index === 0) return null; // Skip the first point

                        const prev = path[index - 1];
                        const start = getCenter(prev.x, prev.y);
                        const end = getCenter(pt.x, pt.y);

                        // Ombre/Gradient Logic:
                        // Base opacity is 0.3. As it repeats paths, the opacities stack visually.
                        // Newer paths are also slightly darker.
                        const recencyFactor = index / path.length;
                        const opacity = 0.3 + (0.4 * recencyFactor);

                        return (
                            <line
                                key={index}
                                x1={start.cx}
                                y1={start.cy}
                                x2={end.cx}
                                y2={end.cy}
                                stroke={`rgba(0, 0, 0, ${opacity})`}
                                strokeWidth="12"
                                strokeLinecap="round"
                                className="transition-all duration-500 ease-in-out"
                            />
                        );
                    })}
                </svg>

                {/* 3. Robot Marker */}
                <div
                    className="absolute w-10 h-10 bg-green-400 border-2 border-green-600 rounded-lg shadow-md flex items-center justify-center transition-all duration-500 ease-in-out transform -translate-x-1/2 -translate-y-1/2 z-10"
                    style={{
                        left: getCenter(currentPos.x, currentPos.y).cx,
                        top: getCenter(currentPos.x, currentPos.y).cy,
                    }}
                >
                    {/* You can swap this emoji for a custom SVG of your robot */}
                    🚜
                </div>
            </div>

            {/* Telemetry Readout */}
            <div className="mt-4 flex gap-4 text-sm text-gray-600 font-mono">
                <div>Current X: {currentPos.x}</div>
                <div>Current Y: {currentPos.y}</div>
                <div>Cells Leveled: {new Set(path.map(p => `${p.x},${p.y}`)).size} / 25</div>
            </div>
        </div>
    );
};

export default SandboxTracker;
