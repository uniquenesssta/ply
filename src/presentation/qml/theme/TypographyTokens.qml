pragma Singleton

import QtQuick

QtObject {
    readonly property font display: Qt.font({
        family: TypographyPrimitives.latinFamily,
        pixelSize: TypographyPrimitives.size32,
        weight: TypographyPrimitives.semiBoldWeight
    })

    readonly property font eyebrow: Qt.font({
        family: TypographyPrimitives.latinFamily,
        pixelSize: TypographyPrimitives.size12,
        weight: TypographyPrimitives.semiBoldWeight
    })

    readonly property font supportingBody: Qt.font({
        family: TypographyPrimitives.cjkFamily,
        pixelSize: TypographyPrimitives.size14,
        weight: TypographyPrimitives.regularWeight
    })

    readonly property font inspectorTitle: Qt.font({
        family: TypographyPrimitives.cjkFamily,
        pixelSize: TypographyPrimitives.size22,
        weight: TypographyPrimitives.mediumWeight
    })

    readonly property font mediaTitle: Qt.font({
        family: TypographyPrimitives.cjkFamily,
        pixelSize: TypographyPrimitives.size14,
        weight: TypographyPrimitives.mediumWeight
    })

    readonly property font mediaTitleCompact: Qt.font({
        family: TypographyPrimitives.cjkFamily,
        pixelSize: TypographyPrimitives.size13,
        weight: TypographyPrimitives.mediumWeight
    })

    readonly property font controlBody: Qt.font({
        family: TypographyPrimitives.cjkFamily,
        pixelSize: TypographyPrimitives.size12,
        weight: TypographyPrimitives.regularWeight
    })

    readonly property font metaBody: Qt.font({
        family: TypographyPrimitives.cjkFamily,
        pixelSize: TypographyPrimitives.size11,
        weight: TypographyPrimitives.regularWeight
    })

    readonly property font technicalMetadata: Qt.font({
        family: TypographyPrimitives.latinFamily,
        pixelSize: TypographyPrimitives.size10,
        weight: TypographyPrimitives.regularWeight
    })

    readonly property font microStrong: Qt.font({
        family: TypographyPrimitives.latinFamily,
        pixelSize: TypographyPrimitives.size10,
        weight: TypographyPrimitives.semiBoldWeight
    })

    readonly property font compactStrong: Qt.font({
        family: TypographyPrimitives.latinFamily,
        pixelSize: TypographyPrimitives.size11,
        weight: TypographyPrimitives.semiBoldWeight
    })

    readonly property font keycapCompact: Qt.font({
        family: TypographyPrimitives.latinFamily,
        pixelSize: TypographyPrimitives.size11,
        weight: TypographyPrimitives.mediumWeight
    })

    readonly property font timecodeMediumPrimary: Qt.font({
        family: TypographyPrimitives.monoFamily,
        pixelSize: TypographyPrimitives.size12,
        weight: TypographyPrimitives.mediumWeight
    })

    readonly property font timecodeMediumSecondary: Qt.font({
        family: TypographyPrimitives.monoFamily,
        pixelSize: TypographyPrimitives.size12,
        weight: TypographyPrimitives.regularWeight
    })

    readonly property font timecodeSmallPrimary: Qt.font({
        family: TypographyPrimitives.monoFamily,
        pixelSize: TypographyPrimitives.size11,
        weight: TypographyPrimitives.mediumWeight
    })

    readonly property font timecodeSmallSecondary: Qt.font({
        family: TypographyPrimitives.monoFamily,
        pixelSize: TypographyPrimitives.size11,
        weight: TypographyPrimitives.regularWeight
    })

    readonly property font timecodeExtraSmall: Qt.font({
        family: TypographyPrimitives.monoFamily,
        pixelSize: TypographyPrimitives.size10,
        weight: TypographyPrimitives.regularWeight
    })
}
