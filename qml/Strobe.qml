import QtQuick 2.0

Item {
    property real diskAngle: 0

    Rectangle {
        id: container
        anchors.fill: parent
        color: "black"

        Image {
            id: disk
            source: "/images/disk"
            width: container.width
            height: container.width
            fillMode: Image.PreserveAspectFit
        }

        ShaderEffect {
            property real intensity: 0
            property variant angle: diskAngle

            property variant source: ShaderEffectSource {
                sourceItem: disk
                hideSource: true
            }

            id: opacityEffect

            anchors {
                horizontalCenter: parent.horizontalCenter
                verticalCenter: parent.bottom
            }
            width: parent.width
            height: parent.width


            fragmentShader: "
                varying highp vec2 qt_TexCoord0;
                uniform float intensity;
                uniform float angle;
                uniform sampler2D source;

                // Roatation matrix
                mat2 rotate(float a) {
                    float s = sin(a);
                    float c = cos(a);
                    return mat2(c, -s, s, c);
                }

                // Find if vector v is within (0,0) - (1,1) bounds
                // 1 - in bounds, 0 otherwise
                float bounds_coef(vec2 v) {
                    vec2 in_bounds = step(v, vec2(0,0)) - step(v, vec2(1,1));
                    // Multiply x and y values, if something out of bounds it will return 0
                    return in_bounds.x*in_bounds.y;
                }

                void main() {
                    // Fancy black out towards the x edges
                    float offset = 1.0 - abs(qt_TexCoord0.x-0.5)*1.8;

                    // Rotation relative to center
                    vec2 c_coord = qt_TexCoord0 - vec2(0.5,0.5);                      // Coord releative to center
                    vec2 a_coord = rotate(radians(angle)) * c_coord + vec2(0.5,0.5);  // Coord rotated and transposed back

                    // Render everything out of bound black by multiplying to <<in bounds coefficient>>
                    vec4 src = texture2D(source, a_coord) * bounds_coef(a_coord);

                    // Fancy highlight colors because why not
                    gl_FragColor = src * vec4(0.8, 0.5, 0.1, 0) * intensity * offset;
                }"
        }
    }

    Timer {
        id: opacityTimer
        onTriggered: {
            var o = opacityEffect.intensity - 0.15
            if (o > 0) {
                opacityEffect.intensity = o;
            } else {
                opacityEffect.intensity = 0;
                stop();
            }
        }
        running: false
        interval: 30
        repeat: true
    }

    onDiskAngleChanged: {
        opacityEffect.intensity = 1.0
        opacityTimer.restart()
    }
}
