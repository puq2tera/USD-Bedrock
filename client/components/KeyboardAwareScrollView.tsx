import { useCallback, useEffect, useMemo, useRef, useState } from "react";
import {
  Dimensions,
  Keyboard,
  KeyboardEvent,
  Platform,
  ScrollView,
  ScrollViewProps,
  StyleSheet,
  StyleProp,
  TextInput,
  ViewStyle,
} from "react-native";
import { useSafeAreaInsets } from "react-native-safe-area-context";

type KeyboardAwareScrollViewProps = Omit<ScrollViewProps, "contentContainerStyle"> & {
  contentContainerStyle?: StyleProp<ViewStyle>;
  extraBottomPadding?: number;
  focusRevealPadding?: number;
  enableOnWeb?: boolean;
};

export function KeyboardAwareScrollView(props: KeyboardAwareScrollViewProps) {
  const {
    contentContainerStyle,
    extraBottomPadding = 12,
    focusRevealPadding = 20,
    enableOnWeb = false,
    onScroll,
    scrollEventThrottle,
    ...rest
  } = props;

  const insets = useSafeAreaInsets();
  const scrollRef = useRef<ScrollView>(null);
  const lastScrollYRef = useRef(0);
  const keyboardHeightRef = useRef(0);
  const [keyboardInset, setKeyboardInset] = useState(0);

  const shouldHandleKeyboard = Platform.OS !== "web" || enableOnWeb;

  const revealFocusedInputIfNeeded = useCallback(() => {
    if (!shouldHandleKeyboard || keyboardHeightRef.current <= 0) {
      return;
    }

    const focusedInput = TextInput.State.currentlyFocusedInput?.();
    if (!focusedInput || typeof focusedInput.measureInWindow !== "function") {
      return;
    }

    focusedInput.measureInWindow((_x: number, y: number, _width: number, height: number) => {
      const viewportHeight = Dimensions.get("window").height;
      const coveredTop = viewportHeight - keyboardHeightRef.current;
      const inputBottom = y + height;
      const maxVisibleBottom = coveredTop - focusRevealPadding;
      if (inputBottom <= maxVisibleBottom) {
        return;
      }

      // Move just enough to keep the focused control visible without large jumps.
      const delta = inputBottom - maxVisibleBottom;
      scrollRef.current?.scrollTo({
        y: Math.max(0, lastScrollYRef.current + delta),
        animated: true,
      });
    });
  }, [focusRevealPadding, shouldHandleKeyboard]);

  const applyKeyboardHeight = useCallback((height: number) => {
    const normalizedHeight = Math.max(0, height);
    keyboardHeightRef.current = normalizedHeight;
    const bottomInsetCompensation = Platform.OS === "ios" ? insets.bottom : 0;
    const nextInset = normalizedHeight > 0
      ? Math.max(0, normalizedHeight - bottomInsetCompensation) + extraBottomPadding
      : extraBottomPadding;
    setKeyboardInset(nextInset);
  }, [extraBottomPadding, insets.bottom]);

  useEffect(() => {
    if (!shouldHandleKeyboard) {
      return;
    }

    const showEvent = Platform.OS === "ios" ? "keyboardWillShow" : "keyboardDidShow";
    const hideEvent = Platform.OS === "ios" ? "keyboardWillHide" : "keyboardDidHide";
    const changeFrameEvent = Platform.OS === "ios" ? "keyboardWillChangeFrame" : "keyboardDidChangeFrame";

    const onShow = (event: KeyboardEvent) => {
      applyKeyboardHeight(event.endCoordinates.height);
      setTimeout(revealFocusedInputIfNeeded, 30);
    };
    const onHide = () => {
      applyKeyboardHeight(0);
    };
    const onFrameChange = (event: KeyboardEvent) => {
      applyKeyboardHeight(event.endCoordinates.height);
      setTimeout(revealFocusedInputIfNeeded, 30);
    };

    const showSub = Keyboard.addListener(showEvent, onShow);
    const hideSub = Keyboard.addListener(hideEvent, onHide);
    const changeFrameSub = Keyboard.addListener(changeFrameEvent, onFrameChange);

    const dimensionsSub = Dimensions.addEventListener("change", () => {
      if (keyboardHeightRef.current > 0) {
        setTimeout(revealFocusedInputIfNeeded, 30);
      }
    });

    return () => {
      showSub.remove();
      hideSub.remove();
      changeFrameSub.remove();
      dimensionsSub.remove();
    };
  }, [applyKeyboardHeight, revealFocusedInputIfNeeded, shouldHandleKeyboard]);

  const baseContentPaddingBottom = useMemo(() => {
    const flattened = StyleSheet.flatten(contentContainerStyle) as ViewStyle | undefined;
    const paddingBottom = flattened?.paddingBottom;
    return typeof paddingBottom === "number" ? paddingBottom : 0;
  }, [contentContainerStyle]);

  return (
    <ScrollView
      ref={scrollRef}
      {...rest}
      keyboardDismissMode={Platform.OS === "ios" ? "interactive" : "on-drag"}
      keyboardShouldPersistTaps="handled"
      scrollEventThrottle={scrollEventThrottle ?? 16}
      onScroll={(event) => {
        lastScrollYRef.current = event.nativeEvent.contentOffset.y;
        onScroll?.(event);
      }}
      contentContainerStyle={[contentContainerStyle, { paddingBottom: baseContentPaddingBottom + keyboardInset }]}
    />
  );
}
