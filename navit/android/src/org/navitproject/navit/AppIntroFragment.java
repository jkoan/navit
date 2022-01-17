package org.navitproject.navit;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import androidx.annotation.Nullable;
import androidx.leanback.app.OnboardingSupportFragment;

public class AppIntroFragment extends OnboardingSupportFragment {
    @Override
    protected int getPageCount() {
        return 0;
    }

    @Override
    protected CharSequence getPageTitle(int pageIndex) {
        return "Test1";
    }

    @Override
    protected CharSequence getPageDescription(int pageIndex) {
        return "bla blub";
    }

    @Nullable
    @Override
    protected View onCreateBackgroundView(LayoutInflater inflater, ViewGroup container) {
        return null;
    }

    @Nullable
    @Override
    protected View onCreateContentView(LayoutInflater inflater, ViewGroup container) {
        return null;
    }

    @Nullable
    @Override
    protected View onCreateForegroundView(LayoutInflater inflater, ViewGroup container) {
        return null;
    }
}
